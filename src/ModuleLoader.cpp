#include "ModuleLoader.hpp"

#include "ziapi/Logger.hpp"

namespace zia {

std::vector<dylib> LoadDynamicLibsFromDir(const std::filesystem::path &lib_path)
{
    std::vector<dylib> libs;

    if (!std::filesystem::is_directory(lib_path)) {
        ziapi::Logger::Warning("Could not load libs from ", lib_path, " : not a directory");
        return {};
    }
    for (auto &file : std::filesystem::recursive_directory_iterator(lib_path)) {
        if (file.path().extension() == dylib::extension) {
            try {
                dylib lib(file.path().string());
                lib.get_function<ziapi::IModule *()>("LoadZiaModule");
                libs.push_back(std::move(lib));
            } catch (const dylib::handle_error &) {
                ziapi::Logger::Warning("Failed to load lib: ", file.path().string());
            } catch (const dylib::symbol_error &) {
                ziapi::Logger::Warning("Failed to load symbol \"LoadZiaModule\" on lib: ", file.path().string());
            }
        }
    }
    return libs;
}

dylib LoadDynamicLib(const std::filesystem::path &lib_path)
{
    try {
        dylib lib(lib_path.string());
        lib.get_function<ziapi::IModule *()>("LoadZiaModule");
        return lib;
    } catch (const dylib::handle_error &) {
        ziapi::Logger::Warning("lib_loader: failed to load: ", lib_path.string());
    } catch (const dylib::symbol_error &) {
        ziapi::Logger::Warning("lib_loader: failed to load symbol \"LoadZiaModule\" on lib: ", lib_path.string());
    }
    throw std::runtime_error("meow");
}

std::vector<dylib> LoadDynamicLibsFromLocations(const std::vector<std::string> &lib_path_list)
{
    std::vector<dylib> libs;
    for (const auto &lib_path : lib_path_list) {
        bool is_dir = false;
        try {
            is_dir = std::filesystem::is_directory(lib_path);
        } catch (...) {
        }
        if (is_dir) {
            auto ret = LoadDynamicLibsFromDir(lib_path);
            for (auto &lib : ret) {
                libs.push_back(std::move(lib));
            }
        } else {
            libs.push_back(LoadDynamicLib(lib_path + dylib::extension));
        }
    }
    return libs;
}

std::vector<std::unique_ptr<ziapi::IModule>> GetModulesFromLibs(const std::vector<dylib> &libs)
{
    std::vector<std::unique_ptr<ziapi::IModule>> mods;
    for (const auto &lib : libs) {
        try {
            auto fn = lib.get_function<ziapi::IModule *()>("LoadZiaModule");
            mods.emplace_back(fn());
        } catch (const dylib::exception &) {
            ziapi::Logger::Warning("Failed to load symbol: LoadZiaModule");
        }
    }
    return mods;
}

std::vector<std::string> GetLibraryLocationsFromConfig(const ziapi::config::Node &cfg)
{
    std::vector<std::string> folders_path;
    auto mod_sources = cfg["locations"].AsArray();

    for (const auto &el : mod_sources) {
        folders_path.push_back(el->AsString());
    }
    return folders_path;
}

}  // namespace zia
