#pragma once

#include <atomic>
#include <memory>
#include <vector>

#include "ModuleAggregate.hpp"
#include "RequestManager.hpp"
#include "TSRequestOutputQueue.hpp"
#include "TSResponseInputQueue.hpp"
#include "ziapi/Http.hpp"
#include "ziapi/Module.hpp"

namespace zia {

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
    RequestManager req_manager_;
    std::thread network_thread_;
    zia::TSRequestOutputQueue requests_;
    zia::TSResponseInputQueue responses_;
    std::atomic<bool> should_stop_;
};

}  // namespace zia
