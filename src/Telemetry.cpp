#include "lcs/Telemetry.hpp"
#include <utility>
namespace lcs::telemetry {
Buffer::Buffer(std::size_t capacity):capacity_(capacity){samples_.reserve(capacity);}
bool Buffer::push(Sample sample){if(capacity_==0||samples_.size()>=capacity_)return false;samples_.push_back(std::move(sample));return true;}
std::vector<Sample> Buffer::drain(){std::vector<Sample> result;result.swap(samples_);return result;}
std::size_t Buffer::size() const noexcept{return samples_.size();}
}
