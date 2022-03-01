#pragma once

#include <memory>
#include <vector>

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

}  // namespace zia