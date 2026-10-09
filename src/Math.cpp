#include "lcs/Math.hpp"
#include <cmath>
#include <stdexcept>
namespace lcs::math {
double magnitude(const Vec3& v){return std::sqrt(v.x*v.x+v.y*v.y+v.z*v.z);}
double clamp(double v,double lo,double hi){if(lo>hi)throw std::invalid_argument("low must be <= high");return v<lo?lo:(v>hi?hi:v);}
double lerp(double a,double b,double t){return a+(b-a)*t;}
}
