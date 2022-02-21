#include "ModulePipeline.hpp"

#include <algorithm>
#include <iostream>

#include "ziapi/Logger.hpp"

namespace zia {

ModuleAggregate ModuleAggregate::From(const std::vector<std::unique_ptr<ziapi::IModule>> &modules)
{
    ziapi::INetworkModule *network_module = nullptr;
    std::vector<std::reference_wrapper<ziapi::IPreProcessorModule>> pre_processors;
    std::vector<std::reference_wrapper<ziapi::IHandlerModule>> handlers;
    std::vector<std::reference_wrapper<ziapi::IPostProcessorModule>> post_processors;

    for (auto &module : modules) {
        auto ptr = module.get();
        auto is_network_module = dynamic_cast<ziapi::INetworkModule *>(ptr);
        auto is_handler = dynamic_cast<ziapi::IHandlerModule *>(ptr);
        auto is_pre = dynamic_cast<ziapi::IPreProcessorModule *>(ptr);
        auto is_post = dynamic_cast<ziapi::IPostProcessorModule *>(ptr);

        if (is_network_module) {
            if (network_module != nullptr) {
                throw std::logic_error("Multiple network module detected, stop it.");
            }
            network_module = is_network_module;
        }
        if (is_pre) {
            pre_processors.emplace_back(*is_pre);
        }
        if (is_handler) {
            handlers.emplace_back(*is_handler);
        }
        if (is_post) {
            post_processors.emplace_back(*is_post);
        }
    }
    std::sort(pre_processors.begin(), pre_processors.end(),
              [](auto &a, auto &b) { return a.get().GetPreProcessorPriority() < b.get().GetPreProcessorPriority(); });
    std::sort(handlers.begin(), handlers.end(),
              [](auto &a, auto &b) { return a.get().GetHandlerPriority() < b.get().GetHandlerPriority(); });
    std::sort(post_processors.begin(), post_processors.end(),
              [](auto &a, auto &b) { return a.get().GetPostProcessorPriority() < b.get().GetPostProcessorPriority(); });

    if (network_module == nullptr) {
        throw std::logic_error("No network module detected, please be provide one.");
    }
    return {*network_module, pre_processors, handlers, post_processors};
}

ModulePipeline::ModulePipeline(const ziapi::config::Node &cfg,
                               const std::vector<std::unique_ptr<ziapi::IModule>> &modules)
    : modules_(ModuleAggregate::From(modules)), network_thread_(), requests_(), responses_(), should_stop_{false}

{
    for (auto &module : modules) {
        module->Init(cfg);
    }
}

void ModulePipeline::Run()
{
    network_thread_ = std::thread([&]() { modules_.network.Run(requests_, responses_); });
    while (true) {
        ziapi::http::Response res{};
        ziapi::http::Request req{};
        ziapi::http::Context ctx{};
        auto try_req = requests_.Pop();

        if (should_stop_)
            break;
        if (!try_req) {
            continue;
        }
        res.Bootstrap();
        std::tie(req, ctx) = try_req.value();

        for (auto &pre_ref : modules_.pre_processors) {
            auto &pre = pre_ref.get();

            if (pre.ShouldPreProcess(ctx, req)) {
                pre.PreProcess(ctx, req);
            }
        }

        for (auto &ref_handler : modules_.handlers) {
            auto &handler = ref_handler.get();

            if (handler.ShouldHandle(ctx, req)) {
                handler.Handle(ctx, req, res);
            }
        }

        for (auto &ref_post : modules_.post_processors) {
            auto &post = ref_post.get();

            if (post.ShouldPostProcess(ctx, res)) {
                post.PostProcess(ctx, res);
            }
        }

        responses_.Push(std::make_pair(res, ctx));
    }
}

void ModulePipeline::Terminate()
{
    should_stop_ = true;
    modules_.network.Terminate();
    network_thread_.join();
    responses_.Clear();
    requests_.Clear();
}

}  // namespace zia
