/*
 */
#include "params.h"

template<class T> std::vector<std::string> split(const std::string& s, const T& separator, bool ignore_empty = 0, bool split_empty = 0) {
  struct {
    auto len(const std::string&             s) { return s.length(); }
    auto len(const std::string::value_type* p) { return p ? std::char_traits<std::string::value_type>::length(p) : 0; }
    auto len(const std::string::value_type  c) { return c == std::string::value_type() ? 0 : 1; /*return 1;*/ }
  } util;
  
  if (s.empty()) { /// empty string ///
    if (!split_empty || util.len(separator)) return {""};
    return {};
  }
  
  auto v = std::vector<std::string>();
  auto n = static_cast<std::string::size_type>(util.len(separator));
  if (n == 0) {    /// empty separator ///
    if (!split_empty) return {s};
    for (auto&& c : s) v.emplace_back(1, c);
    return v;
  }
  
  auto p = std::string::size_type(0);
  while (1) {      /// split with separator ///
    auto pos = s.find(separator, p);
    if (pos == std::string::npos) {
      if (ignore_empty && p - n + 1 == s.size()) break;
      v.emplace_back(s.begin() + p, s.end());
      break;
    }
    if (!ignore_empty || p != pos)
      v.emplace_back(s.begin() + p, s.begin() + pos);
    p = pos + n;
  }
  return v;
}


YAML::Node 
get_node(YAML::Node node, const char *path)
{
  std::vector<std::string> ppath_=split(path,'/');
  YAML::Node res=node[ppath_[0].c_str()];

  int len=ppath_.size();

  if(len == 1) return node;
  for(int i=1; i < len-1; i++){
    res=res[ppath_[i].c_str()];
  }
  return  res;
}

const char*
get_path_name(const char *path)
{
  std::vector<std::string> ppath_=split(path,'/');
  int last=ppath_.size() -1;
  return ppath_[last].c_str();
}

std::string
get_param_string(YAML::Node node, const char *name, std::string defval)
{
  try{
    YAML::Node node_=get_node(node, name);
    return node_[get_path_name(name)].as<std::string>();
  } catch (...){
    std::cout << "Error in convertion: string" << std::endl;
  }

  return defval;
}

int
get_param_int(YAML::Node node, const char *name, int defval)
{
  try{
    YAML::Node node_=get_node(node, name);
    return node_[get_path_name(name)].as<int>();
  } catch (...){
    std::cout << "Error in convertion: int" << std::endl;
  }

  return defval;
}

