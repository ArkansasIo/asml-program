#pragma once
#include "lcs/Telemetry.hpp"
#include <string>
#include <vector>
namespace lcs::persistence {
class TelemetryRepository { public: virtual ~TelemetryRepository()=default; virtual bool append(const lcs::telemetry::Sample&,std::string& error)=0; virtual std::vector<lcs::telemetry::Sample> recent(std::uint64_t sensorId,std::size_t limit,std::string& error)=0; };
class NullTelemetryRepository final: public TelemetryRepository { public: bool append(const lcs::telemetry::Sample&,std::string&) override{return true;} std::vector<lcs::telemetry::Sample> recent(std::uint64_t,std::size_t,std::string&) override{return {};} };
}
