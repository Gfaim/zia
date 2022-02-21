#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "TSRequestOutputQueue.hpp"
#include "TSResponseInputQueue.hpp"
#include "ziapi/Http.hpp"
#include "ziapi/Module.hpp"

namespace zia {

/// Aggregation of all the modules of a pipeline.
struct ModuleAggregate {
    ziapi::INetworkModule &network;

    std::vector<std::reference_wrapper<ziapi::IPreProcessorModule>> pre_processors;

    std::vector<std::reference_wrapper<ziapi::IHandlerModule>> handlers;

    std::vector<std::reference_wrapper<ziapi::IPostProcessorModule>> post_processors;

    /// Create a module bundle from a vector of modules by dynamically casting
    /// each module into the right category.
    static ModuleAggregate From(const std::vector<std::unique_ptr<ziapi::IModule>> &modules);
};

/// Manages an execution context for requests and responses.
class ModulePipeline {
public:
    /// Constructs the object from all the modules that make up the pipeline.
    /// If there are multiple INetworkModules in the modules vector, this
    /// constructor will throw.
    ModulePipeline(const ziapi::config::Node &cfg, const std::vector<std::unique_ptr<ziapi::IModule>> &modules);

    ModulePipeline(const ModulePipeline &) = delete;
    ModulePipeline &operator=(const ModulePipeline &) = delete;

    /// Start running the pipeline. This call is blocking.
    void Run();

    /// Terminate the pipeline by gracefully terminating the network module and
    /// all other running modules.
    /// This call is thread safe.
    void Terminate();

private:
    ModuleAggregate modules_;
    std::thread network_thread_;
    zia::TSRequestOutputQueue requests_;
    zia::TSResponseInputQueue responses_;
    std::atomic<bool> should_stop_;
};

}  // namespace zia
