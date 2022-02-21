#pragma once

#include <memory>

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

    void Restart();

    void CleanUp();

private:
    std::vector<dylib> libs_;

    std::vector<std::unique_ptr<ziapi::IModule>> modules_;

    ModulePipeline pipeline_;

    std::thread pipeline_thread_;

    Params args_;

    ziapi::config::Node cfg_;
};

}  // namespace zia
