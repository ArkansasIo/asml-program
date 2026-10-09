#include "lcs/Configuration.hpp"
#include <utility>
namespace lcs::config {
void Configuration::set(std::string k,std::string v){values_[std::move(k)]=std::move(v);}
std::string Configuration::get(const std::string& k,const std::string& f)const{auto i=values_.find(k);return i==values_.end()?f:i->second;}
}
