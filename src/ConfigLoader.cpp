#include "ConfigLoader.hpp"

#include <iostream>

#include "config/ParseYAML.hpp"

namespace zia {

ziapi::config::Node LoadConfig(const std::filesystem::path &path)
{
    return zia::config::ParseYAMLFromFile(std::string(path));
}

}  // namespace zia
