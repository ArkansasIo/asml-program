// Winsock2 implementation of the LCS REST API HTTP server.
#include "lcs/HttpServer.hpp"

#include <winsock2.h>
#include <ws2tcpip.h>

#include <cctype>
#include <iostream>
#include <sstream>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

namespace lcs::api
{
    namespace
    {

        std::string statusText(int code)
        {
            switch (code)
            {
            case 200:
                return "OK";
            case 400:
                return "Bad Request";
            case 404:
                return "Not Found";
            case 405:
                return "Method Not Allowed";
            case 500:
                return "Internal Server Error";
            default:
                return "OK";
            }
        }

        // Very small JSON string escaper for responses we build.
        std::string jsonEscape(const std::string &in)
        {
            std::string out;
            out.reserve(in.size());
            for (char c : in)
            {
                switch (c)
                {
                case '"':
                    out += "\\\"";
                    break;
                case '\\':
                    out += "\\\\";
                    break;
                case '\n':
                    out += "\\n";
                    break;
                case '\r':
                    out += "\\r";
                    break;
                case '\t':
                    out += "\\t";
                    break;
                default:
                    if (static_cast<unsigned char>(c) < 0x20)
                    {
                        char buf[8];
                        std::snprintf(buf, sizeof(buf), "\\u%04x", c);
                        out += buf;
                    }
                    else
                    {
                        out += c;
                    }
                }
            }
            return out;
        }

    } // namespace

    HttpServer::HttpServer(unsigned short port, HttpHandler handler)
        : port_(port), handler_(std::move(handler)) {}

    HttpServer::~HttpServer() = default;

    void HttpServer::serve()
    {
        WSADATA wsa{};
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        {
            std::cerr << "WSAStartup failed\n";
            return;
        }

        SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listener == INVALID_SOCKET)
        {
            std::cerr << "socket() failed\n";
            WSACleanup();
            return;
        }

        BOOL reuse = TRUE;
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char *>(&reuse), sizeof(reuse));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // localhost only
        addr.sin_port = htons(port_);

        if (bind(listener, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == SOCKET_ERROR ||
            listen(listener, SOMAXCONN) == SOCKET_ERROR)
        {
            std::cerr << "bind/listen failed on port " << port_ << "\n";
            closesocket(listener);
            WSACleanup();
            return;
        }

        std::cout << "LCS API listening on http://127.0.0.1:" << port_ << "\n";

        for (;;)
        {
            SOCKET client = accept(listener, nullptr, nullptr);
            if (client == INVALID_SOCKET)
                continue;

            std::thread([client, handler = handler_]
                        {
            char buf[8192];
            int received = 0;
            std::string raw;
            // Read headers (+ small body) until shutdown or buffer is enough.
            for (;;) {
                int n = recv(client, buf, sizeof(buf), 0);
                if (n <= 0) break;
                raw.append(buf, static_cast<size_t>(n));
                if (raw.find("\r\n\r\n") != std::string::npos) break;
                received += n;
                if (received > 1 << 20) break;  // 1 MiB cap
            }

            HttpRequest req;
            {
                std::istringstream iss(raw);
                std::string line;
                if (std::getline(iss, line) && line.size() > 8) {
                    std::istringstream first(line);
                    first >> req.method >> req.path;
                }
                // keep query string: /api/math/mag parses params from it
            }
            // Determine Content-Length, then read the body fully.
            size_t headerEnd = raw.find("\r\n\r\n");
            size_t contentLength = 0;
            {
                // case-insensitive scan for Content-Length header
                std::string lower;
                lower.reserve(raw.size());
                for (char c : raw) lower.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
                auto pos = lower.find("content-length:");
                if (pos != std::string::npos && pos < headerEnd) contentLength = std::strtoull(raw.c_str() + pos + 15, nullptr, 10);
            }
            if (headerEnd != std::string::npos) {
                req.body = raw.substr(headerEnd + 4);
                while (req.body.size() < contentLength) {
                    int n = recv(client, buf, sizeof(buf), 0);
                    if (n <= 0) break;
                    req.body.append(buf, static_cast<size_t>(n));
                }
            }

            HttpResponse res;
            try {
                res = handler(req);
            } catch (const std::exception& e) {
                res.status = 500;
                res.body = std::string("{\"error\":\"") + jsonEscape(e.what()) + "\"}";
            }

            std::ostringstream out;
            out << "HTTP/1.1 " << res.status << ' ' << statusText(res.status) << "\r\n"
                << "Content-Type: " << res.contentType << "\r\n"
                << "Content-Length: " << res.body.size() << "\r\n"
                << "Connection: close\r\n\r\n"
                << res.body;
            const std::string wire = out.str();
            send(client, wire.data(), static_cast<int>(wire.size()), 0);
            shutdown(client, SD_SEND);
            closesocket(client); })
                .detach();
        }
    }

} // namespace lcs::api
