#pragma once

#include <filesystem>

#include "ziapi/Config.hpp"

namespace zia {

ziapi::config::Node LoadConfig(const std::filesystem::path &path);

}  // namespace zia
