#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "dylib/dylib.hpp"
#include "ziapi/Module.hpp"

namespace zia {

std::vector<dylib> LoadDynamicLibsFromDir(const std::filesystem::path &lib_path);

std::vector<dylib> LoadDynamicLibsFromLocations(const std::vector<std::string> &lib_path_list);

std::vector<std::unique_ptr<ziapi::IModule>> GetModulesFromLibs(const std::vector<dylib> &libs);

std::vector<std::string> GetLibraryLocationsFromConfig(const ziapi::config::Node &cfg);

}  // namespace zia
