// LCS REST API server (simulation backend, physical control disabled).
// Endpoints:
//   GET  /api/state                 -> machine snapshot
//   POST /api/init                  -> initialize machine
//   POST /api/start                 -> start simulation
//   POST /api/pause                 -> pause simulation
//   POST /api/stop                  -> simulation
//   POST /api/move  {"x":..,"y":..,"z":..} -> move stage
//   GET  /api/math/mag?x=&y=&z=     -> vector magnitude
//   GET  /                          -> UI (ui/index.html)
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "lcs/ControlPlanes.hpp"
#include "lcs/HttpServer.hpp"
#include "lcs/Math.hpp"
#include "lcs/VirtualMachine.hpp"

namespace
{

    std::string stateName(lcs::simulation::State s)
    {
        switch (s)
        {
        case lcs::simulation::State::Offline:
            return "Offline";
        case lcs::simulation::State::Ready:
            return "Ready";
        case lcs::simulation::State::Running:
            return "Running";
        case lcs::simulation::State::Paused:
            return "Paused";
        case lcs::simulation::State::Fault:
            return "Fault";
        }
        return "Unknown";
    }

    std::string jsonResponse(int status, const std::string &body)
    {
        lcs::api::HttpResponse r;
        r.status = status;
        r.body = body;
        return {}; // unreachable; kept for clarity
    }

    bool readNumber(const std::string &body, const char *key, double &value)
    {
        // Lightweight parse: find "key" then a number after ':'.
        std::string needle = std::string("\"") + key + "\"";
        auto pos = body.find(needle);
        if (pos == std::string::npos)
            return false;
        pos = body.find(':', pos + needle.size());
        if (pos == std::string::npos)
            return false;
        ++pos;
        while (pos < body.size() && std::isspace(static_cast<unsigned char>(body[pos])))
            ++pos;
        char *end = nullptr;
        value = std::strtod(body.c_str() + pos, &end);
        return end != body.c_str() + pos && std::isfinite(value);
    }

    double queryNumber(const std::string &path, const char *key, double fallback)
    {
        auto pos = path.find(std::string(key) + "=");
        if (pos == std::string::npos)
            return fallback;
        pos += std::string(key).size() + 1;
        return std::strtod(path.c_str() + pos, nullptr);
    }

    std::string readFile(const std::string &path)
    {
        std::ifstream in(path, std::ios::binary);
        if (!in)
            return {};
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }

    std::string contentTypeFor(const std::string &path)
    {
        if (path.size() >= 5 && path.rfind(".html") == path.size() - 5)
            return "text/html";
        if (path.size() >= 4 && path.rfind(".css") == path.size() - 4)
            return "text/css";
        if (path.size() >= 3 && path.rfind(".js") == path.size() - 3)
            return "text/javascript";
        return "application/octet-stream";
    }

} // namespace

int main(int argc, char **argv)
{
    unsigned short port = argc > 1 ? static_cast<unsigned short>(std::atoi(argv[1])) : 8080;
    lcs::control::ControlPlanes planes;

    auto snapshotJson = [&](const lcs::simulation::Snapshot& s) {
        std::ostringstream out;
        out << "{\"state\":\"" << lcs::control::MachineControlPlane::stateName(s.state) << "\""
            << ",\"position\":{\"x\":" << s.position.x << ",\"y\":" << s.position.y << ",\"z\":" << s.position.z << "}"
            << ",\"temperatureK\":" << s.temperatureK
            << ",\"pressurePa\":" << s.pressurePa << "}";
        return out.str();
    };

    lcs::api::HttpServer server(port, [&](const lcs::api::HttpRequest &req) -> lcs::api::HttpResponse
                                {
        lcs::api::HttpResponse res;

        if (req.path == "/" || req.path == "/index.html") {
            res.contentType = "text/html";
            res.body = readFile("ui/index.html");
            if (res.body.empty()) {
                res.status = 404;
                res.body = "<h1>LCS UI not found</h1><p>Run from the project root so ui/ is available.</p>";
            }
            return res;
        }
        if (req.path == "/layout.css" || req.path == "/ui/layout.css") {
            res.contentType = "text/css";
            res.body = readFile("ui/layout.css");
            return res;
        }
        if (req.path == "/app.js" || req.path == "/ui/app.js") {
            res.contentType = "text/javascript";
            res.body = readFile("ui/app.js");
            return res;
        }
        if (req.path == "/dark-industrial.css" || req.path == "/ui/themes/dark-industrial.css") {
            res.contentType = "text/css";
            res.body = readFile("ui/themes/dark-industrial.css");
            return res;
        }

        if (req.path == "/api/state") {
            res.body = snapshotJson(planes.machine().snapshot());
            return res;
        }
        if (req.path == "/api/init") { planes.machine().initialize(); res.body = snapshotJson(planes.machine().snapshot()); return res; }
        if (req.path == "/api/start") { planes.machine().start(); res.body = snapshotJson(planes.machine().snapshot()); return res; }
        if (req.path == "/api/pause") { planes.machine().pause(); res.body = snapshotJson(planes.machine().snapshot()); return res; }
        if (req.path == "/api/stop") { planes.machine().stop(); res.body = snapshotJson(planes.machine().snapshot()); return res; }
        if (req.path == "/api/move") {
            if (req.method != "POST") { res.status = 405; res.body = "{\"error\":\"POST required\"}"; return res; }
            double x = 0, y = 0, z = 0;
            if (!readNumber(req.body, "x", x) || !readNumber(req.body, "y", y) || !readNumber(req.body, "z", z)) {
                res.status = 400;
                res.body = "{\"error\":\"expected JSON body with numeric x, y, z\"}";
                return res;
            }
            const bool ok = planes.stage().moveTo({x, y, z});
            if (!ok) { res.status = 422; res.body = "{\"error\":\"move rejected (limits or interlock)\"}"; return res; }
            const auto& p = planes.stage().position();
            std::ostringstream out;
            out << "{\"position\":{\"x\":" << p.x << ",\"y\":" << p.y << ",\"z\":" << p.z << "}}";
            res.body = out.str();
            return res;
        }

        // ---- control planes ----
        if (req.path == "/api/safety") {
            std::ostringstream out;
            out << "{\"allSatisfied\":" << (planes.safety().allSatisfied() ? "true" : "false") << ",\"failures\":[";
            const auto& f = planes.safety().failures();
            for (std::size_t i = 0; i < f.size(); ++i) out << (i ? ",\"" : "\"" ) << f[i] << "\"";
            out << "]";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/safety/set") {
            // body: {"name":"...","satisfied":true|false}
            std::string name;
            { auto pos = req.body.find("\\\"name\\\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing name\"}"; return res; }
              pos = req.body.find(':', pos); auto start = req.body.find('"', pos); auto end = req.body.find('"', start+1);
              name = req.body.substr(start+1, end-start-1); }
            bool satisfied = req.body.find("true") != std::string::npos;
            planes.safety().update(name, satisfied);
            res.body = planes.safety().allSatisfied() ? "{\"allSatisfied\":true}" : "{\"allSatisfied\":false}";
            return res;
        }
        if (req.path == "/api/optics") {
            const auto& p = planes.optics().parameters();
            std::ostringstream out;
            out << "{\"wavelengthNm\":" << p.wavelengthNm << ",\"numericalAperture\":" << p.numericalAperture
                << ",\"reduction\":" << p.reduction << ",\"resolutionNm\":" << planes.optics().resolutionNm() << "}";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/optics/set") {
            double wl = 0, na = 0, red = 0;
            if (!readNumber(req.body, "wavelengthNm", wl) || !readNumber(req.body, "numericalAperture", na) || !readNumber(req.body, "reduction", red)) {
                res.status = 400; res.body = "{\"error\":\"expected wavelengthNm, numericalAperture, reduction\"}"; return res;
            }
            const bool ok = planes.optics().setParameters({wl, na, red});
            res.status = ok ? 200 : 422;
            res.body = ok ? "{\"accepted\":true}" : "{\"error\":\"invalid optics parameters\"}";
            return res;
        }
        if (req.path == "/api/alarms") {
            std::ostringstream out;
            out << "{\"hasCritical\":" << (planes.alarms().hasCritical() ? "true" : "false") << ",\"active\":[";
            const auto& a = planes.alarms().active();
            for (std::size_t i = 0; i < a.size(); ++i) {
                const char* sev = a[i].severity == lcs::core::Severity::Critical ? "critical"
                                : a[i].severity == lcs::core::Severity::Warning ? "warning" : "info";
                out << (i ? ",{" : "{") << "\"code\":\"" << a[i].code << "\",\"severity\":\"" << sev
                    << "\",\"message\":\"" << a[i].message << "\",\"active\":" << (a[i].active ? "true" : "false") << "}";
            }
            out << "]";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/alarms/raise") {
            std::string code, message, sev;
            { auto pos = req.body.find("\"code\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing code\"}"; return res; }
              pos = req.body.find(':', pos); auto s = req.body.find('"', pos); auto e = req.body.find('"', s+1); code = req.body.substr(s+1, e-s-1); }
            { auto pos = req.body.find("\"message\""); auto s = pos==std::string::npos?std::string::npos:req.body.find('"', req.body.find(':', pos)); auto e = s==std::string::npos?std::string::npos:req.body.find('"', s+1); message = s==std::string::npos?"":req.body.substr(s+1, e-s-1); }
            { auto pos = req.body.find("\"severity\""); auto s = pos==std::string::npos?std::string::npos:req.body.find('"', req.body.find(':', pos)); auto e = s==std::string::npos?std::string::npos:req.body.find('"', s+1); sev = s==std::string::npos?"info":req.body.substr(s+1, e-s-1); }
            const lcs::core::Severity severity = sev == "critical" ? lcs::core::Severity::Critical
                                              : sev == "warning" ? lcs::core::Severity::Warning : lcs::core::Severity::Info;
            planes.alarms().raise(code, severity, message);
            res.body = "{\"raised\":true}";
            return res;
        }
        if (req.path == "/api/alarms/clear") {
            std::string code;
            { auto pos = req.body.find("\"code\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing code\"}"; return res; }
              pos = req.body.find(':', pos); auto s = req.body.find('"', pos); auto e = req.body.find('"', s+1); code = req.body.substr(s+1, e-s-1); }
            const bool ok = planes.alarms().clear(code);
            res.body = ok ? "{\"cleared\":true}" : "{\"cleared\":false}";
            return res;
        }
        if (req.path == "/api/telemetry/push") {
            double sensorId = 0, value = 0;
            if (!readNumber(req.body, "sensorId", sensorId) || !readNumber(req.body, "value", value)) {
                res.status = 400; res.body = "{\"error\":\"expected sensorId, value\"}"; return res;
            }
            std::string unit;
            { auto pos = req.body.find("\"unit\""); auto s = pos==std::string::npos?std::string::npos:req.body.find('"', req.body.find(':', pos)); auto e = s==std::string::npos?std::string::npos:req.body.find('"', s+1); unit = s==std::string::npos?"":req.body.substr(s+1, e-s-1); }
            const bool ok = planes.telemetry().push({static_cast<std::uint64_t>(sensorId), value, unit});
            std::ostringstream out;
            out << "{\"accepted\":" << (ok ? "true" : "false") << ",\"samples\":" << planes.telemetry().size() << "}";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/telemetry/drain") {
            auto samples = planes.telemetry().drain();
            std::ostringstream out;
            out << "[";
            for (std::size_t i = 0; i < samples.size(); ++i) {
                out << (i ? ",{" : "{") << "\"sensorId\":" << samples[i].sensorId << ",\"value\":" << samples[i].value
                    << ",\"unit\":\"" << samples[i].unit << "\"}";
            }
            out << "]";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/jobs/create") {
            std::string id, error;
            { auto pos = req.body.find("\"id\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing id\"}"; return res; }
              pos = req.body.find(':', pos); auto s = req.body.find('"', pos); auto e = req.body.find('"', s+1); id = req.body.substr(s+1, e-s-1); }
            if (!planes.jobs().create(id, error)) { res.status = 422; res.body = "{\"error\":\"" + error + "\"}"; return res; }
            res.body = "{\"created\":true}";
            return res;
        }
        if (req.path.rfind("/api/jobs/", 0) == 0) {
            // generic lifecycle: /api/jobs/<id>/<action> where action in validate|start|complete|cancel
            std::string rest = req.path.substr(std::strlen("/api/jobs/"));
            auto slash = rest.find('/');
            if (slash == std::string::npos) { res.status = 404; res.body = "{\"error\":\"expected /api/jobs/<id>/<action>\"}"; return res; }
            const std::string id = rest.substr(0, slash);
            const std::string action = rest.substr(slash + 1);
            std::string error;
            bool ok = false;
            if (action == "validate") ok = planes.jobs().validate(id, error);
            else if (action == "start") ok = planes.jobs().start(id, error);
            else if (action == "complete") ok = planes.jobs().complete(id, error);
            else if (action == "cancel") ok = planes.jobs().cancel(id, error);
            else { res.status = 404; res.body = "{\"error\":\"unknown job action\"}"; return res; }
            if (!ok) { res.status = 422; res.body = "{\"error\":\"" + error + "\"}"; return res; }
            const auto* job = planes.jobs().find(id);
            const char* state = job ? (job->state() == lcs::job::State::Running ? "Running"
                                    : job->state() == lcs::job::State::Completed ? "Completed"
                                    : job->state() == lcs::job::State::Draft ? "Draft" : "Validated") : "?";
            res.body = std::string("{\"id\":\"") + id + "\",\"state\":\"" + state + "\"}";
            return res;
        }
        if (req.path == "/api/audit") {
            std::ostringstream out;
            out << "[";
            const auto& entries = planes.audit().entries();
            bool first = true;
            for (const auto& e : entries) {
                if (!first) out << ",{"; else { out << "{"; first = false; }
                out << "\"id\":" << e.id << ",\"plane\":\"" << e.plane << "\",\"action\":\"" << e.action
                    << "\",\"detail\":\"" << e.detail << "\"}";
            }
            out << "]";
            res.body = out.str();
            return res;
        }
        if (req.path == "/api/config/set") {
            std::string key, value;
            { auto pos = req.body.find("\"key\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing key\"}"; return res; }
              pos = req.body.find(':', pos); auto s = req.body.find('"', pos); auto e = req.body.find('"', s+1); key = req.body.substr(s+1, e-s-1); }
            { auto pos = req.body.find("\"value\""); auto s = pos==std::string::npos?std::string::npos:req.body.find('"', req.body.find(':', pos)); auto e = s==std::string::npos?std::string::npos:req.body.find('"', s+1); value = s==std::string::npos?"":req.body.substr(s+1, e-s-1); }
            planes.config().set(key, value);
            res.body = "{\"set\":true}";
            return res;
        }
        if (req.path == "/api/config/get") {
            std::string key;
            { auto pos = req.body.find("\"key\""); if (pos == std::string::npos) { res.status=400; res.body="{\"error\":\"missing key\"}"; return res; }
              pos = req.body.find(':', pos); auto s = req.body.find('"', pos); auto e = req.body.find('"', s+1); key = req.body.substr(s+1, e-s-1); }
            const std::string value = planes.config().get(key);
            res.body = "{\"key\":\"" + key + "\",\"value\":\"" + value + "\"}";
            return res;
        }
        if (req.path.rfind("/api/math/mag", 0) == 0) {
            double x = queryNumber(req.path, "x", 0);
            double y = queryNumber(req.path, "y", 0);
            double z = queryNumber(req.path, "z", 0);
            std::ostringstream out;
            out << "{\"magnitude\":" << lcs::math::magnitude({x, y, z}) << "}";
            res.body = out.str();
            return res;
        }

        res.status = 404;
        res.body = "{\"error\":\"not found\"}";
        return res; });

    server.serve();
    return 0;
}
