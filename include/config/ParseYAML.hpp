#pragma once

#include <filesystem>

#include "ziapi/Config.hpp"
#include "yaml-cpp/yaml.h"

namespace zia::config {

using Node = ziapi::config::Node;

Node ParseYAML(const std::string &content);

Node ParseYAMLFromFile(const std::filesystem::path &file);

};  // namespace zia::config
