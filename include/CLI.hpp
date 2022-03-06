#pragma once

#include <memory>

#include "FileWatcher.hpp"
#include "ModuleLoader.hpp"
#include "ModulePipeline.hpp"
#include "Params.hpp"
#include "Zia.hpp"
#include "ziapi/Config.hpp"
#include "ziapi/Module.hpp"

namespace zia {

class CLI {
public:
    CLI(const zia::Params &params, const ziapi::config::Node &cfg);

    ~CLI();

private:
    void StartModulePipeline();

    void Run();

    void HandleInput();

    // void HotReload();

    void Restart();

    void CleanUp();

private:
    std::vector<dylib> libs_;

    std::vector<std::unique_ptr<ziapi::IModule>> modules_;

    std::unique_ptr<ModulePipeline> pipeline_;

    std::thread pipeline_thread_;

    std::thread input_thread_;

    // std::thread hot_reload_thread_;

    // FileWatcher watcher_;

    Params args_;

    ziapi::config::Node cfg_;

    std::atomic<bool> should_restart_;
};

}  // namespace zia
