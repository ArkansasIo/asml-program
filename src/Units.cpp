#include "lcs/Units.hpp"
namespace lcs::units {
double toMillimeters(double v, Length u) noexcept { switch(u){case Length::Meter:return v*1000.0;case Length::Millimeter:return v;case Length::Micrometer:return v/1000.0;case Length::Nanometer:return v/1'000'000.0;} return v; }
double toNanometers(double v, Length u) noexcept { return toMillimeters(v,u)*1'000'000.0; }
double toPascal(double v, Pressure u) noexcept { return u==Pressure::KiloPascal?v*1000.0:v; }
double toKelvin(double v, Temperature u) noexcept { return u==Temperature::Celsius?v+273.15:v; }
std::string_view name(Length u) noexcept { switch(u){case Length::Meter:return "m";case Length::Millimeter:return "mm";case Length::Micrometer:return "um";case Length::Nanometer:return "nm";} return ""; }
}
