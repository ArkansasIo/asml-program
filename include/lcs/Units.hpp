#pragma once
#include <string_view>
namespace lcs::units {
enum class Length { Meter, Millimeter, Micrometer, Nanometer };
enum class Pressure { Pascal, KiloPascal };
enum class Temperature { Kelvin, Celsius };
double toMillimeters(double value, Length unit) noexcept;
double toNanometers(double value, Length unit) noexcept;
double toPascal(double value, Pressure unit) noexcept;
double toKelvin(double value, Temperature unit) noexcept;
std::string_view name(Length unit) noexcept;
}
