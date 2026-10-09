#pragma once
namespace lcs::math {
struct Vec3 { double x{},y{},z{}; constexpr Vec3 operator+(const Vec3& b) const { return {x+b.x,y+b.y,z+b.z}; } };
double magnitude(const Vec3&); double clamp(double,double,double); double lerp(double,double,double);
}
