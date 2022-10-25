/*
 */

#include <yaml-cpp/yaml.h>
#include <iostream>
#include <string>
#include <vector>

YAML::Node get_node(YAML::Node node, const char *path);

std::string get_param_string(YAML::Node node, const char *name, std::string defval);
int get_param_int(YAML::Node node, const char *name, int defval);
