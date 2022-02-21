#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#include "dylib/dylib.hpp"
#include "ziapi/Module.hpp"

namespace zia {

std::vector<dylib> LoadDynamicLibsFromDir(const std::filesystem::path &lib_path);

std::vector<dylib> LoadDynamicLibsFromDirs(const std::vector<std::filesystem::path> &lib_path_list);

std::vector<std::unique_ptr<ziapi::IModule>> GetModulesFromLibs(const std::vector<dylib> &libs);

std::vector<std::filesystem::path> GetLibraryPathFromCfg(const ziapi::config::Node &cfg);

}  // namespace zia
