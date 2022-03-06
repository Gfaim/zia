#include "CLI.hpp"

#include <iostream>
#include <thread>
#include <ziapi/Logger.hpp>

namespace zia {

CLI::CLI(const zia::Params &args, const ziapi::config::Node &cfg)
    : libs_(zia::LoadDynamicLibsFromDirs(zia::GetLibraryPathFromCfg(cfg))),
      modules_(zia::GetModulesFromLibs(libs_)),
      pipeline_(std::make_unique<ModulePipeline>(cfg, modules_, args.num_threads)),
      pipeline_thread_(),
      input_thread_(),
      hot_reload_thread_(),
      pipeline_mutex_(),
      watcher_(zia::GetLibraryPathFromCfg(cfg)),
      args_(args),
      cfg_(cfg),
      should_restart_()
{
    for (auto &mod : modules_) {
        ziapi::Logger::Info("using module: ", mod->GetName(), " v", mod->GetVersion().major, ".",
                            mod->GetVersion().minor, ".", mod->GetVersion().patch);
    }
    StartModulePipeline();
    Run();
}

void CLI::StartModulePipeline()
{
    pipeline_thread_ = std::thread([&]() { pipeline_->Run(); });
    hot_reload_thread_ = std::thread([&]() { this->HotReload(); });
    input_thread_ = std::thread([&]() { this->HandleInput(); });
}

void CLI::HotReload()
{
    while (!should_restart_) {
        if (watcher_.HasChanged()) {
            should_restart_ = true;
            watcher_.Update();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void CLI::HandleInput()
{
    std::string input;

    while (std::getline(std::cin, input)) {
        if (input == "q") {
            break;
        }
        if (input == "r" or input == "restart") {
            should_restart_ = true;
        }
    }
}

void CLI::Run()
{
    while (true) {
        if (should_restart_) {
            Restart();
        }
    }
}

void CLI::CleanUp()
{
    // Clear all threads
    pipeline_->Terminate();
    pipeline_thread_.join();
    hot_reload_thread_.join();
}

void CLI::Restart()
{
    // Reload, ...
    ziapi::Logger::Debug("Reload...");
    // recreate pipeline obj;
    CleanUp();
    modules_.clear();
    libs_ = zia::LoadDynamicLibsFromDirs(zia::GetLibraryPathFromCfg(cfg_));
    ziapi::Logger::Debug("Getting modules from libraries");
    modules_ = zia::GetModulesFromLibs(libs_);
    for (auto &mod : modules_) {
        ziapi::Logger::Info("using module: ", mod->GetName(), " v", mod->GetVersion().major, ".",
                            mod->GetVersion().minor, ".", mod->GetVersion().patch);
    }
    ziapi::Logger::Debug("Instantiating Module pipeline...");
    pipeline_ = std::make_unique<ModulePipeline>(cfg_, modules_, args_.num_threads);
    pipeline_thread_ = std::thread([&]() { pipeline_->Run(); });
    hot_reload_thread_ = std::thread([&]() { this->HotReload(); });
    should_restart_ = false;
    ziapi::Logger::Debug("Successfully restarted!");
}

CLI::~CLI() { CleanUp(); }

}  // namespace zia
