#include "CLI.hpp"

#include <iostream>
#include <thread>

namespace zia {

CLI::CLI(const zia::Params &args, const ziapi::config::Node &cfg)
    : libs_(zia::LoadDynamicLibsFromDirs(zia::GetLibraryPathFromCfg(cfg))),
      modules_(zia::GetModulesFromLibs(libs_)),
      pipeline_(cfg, modules_, args.num_threads),
      pipeline_thread_(),
      args_(args),
      cfg_(cfg)
{
    for (auto &mod : modules_) {
        std::cout << "Module: " << mod->GetName() << std::endl;
    }
    StartModulePipeline();
    Run();
}

void CLI::StartModulePipeline()
{
    pipeline_thread_ = std::thread([&]() { pipeline_.Run(); });
}

void CLI::Run()
{
    std::string input;

    while (std::getline(std::cin, input)) {
        if (input == "q") {
            break;
        }
        if (input == "r") {
            Restart();
        }
    }
}

void CLI::CleanUp()
{
    // Clear all threads
    pipeline_.Terminate();
    pipeline_thread_.join();
}

void CLI::Restart()
{
    // Reload, ...
    CleanUp();
    // recreate pipeline obj;
}

CLI::~CLI() { CleanUp(); }

}  // namespace zia
