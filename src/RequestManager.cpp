#include "RequestManager.hpp"

#include <thread>

namespace zia {

RequestManager::RequestManager(int max_threads, zia::ModuleAggregate &mods) : m_mods(mods), m_threads(max_threads)
{
    for (int i = 0; i < max_threads; i++) m_threads[i] = std::thread(Worker, this);
}

RequestManager::~RequestManager()
{
    Clear();
    m_dtor = true;
    for (auto &th : m_threads) {
        if (th.joinable())
            th.join();
    }
}

void RequestManager::AddRequest(std::pair<ziapi::http::Request, ziapi::http::Context> request)
{
    std::scoped_lock lck(m_req_lock_guard);
    m_req.push_back(request);
}

std::vector<std::pair<ziapi::http::Response, ziapi::http::Context>> RequestManager::PopResponses()
{
    std::scoped_lock lck(m_res_lock_guard);
    auto res = std::move(m_res);
    return res;
}

void RequestManager::Clear()
{
    std::scoped_lock lck(m_req_lock_guard);
    m_req.clear();
}

void RequestManager::Worker(RequestManager *self)
{
    while (true) {
        std::this_thread::yield();
        std::optional<std::pair<ziapi::http::Request, ziapi::http::Context>> m_current_req = std::nullopt;
        ziapi::http::Response res{};
        ziapi::http::Request req{};
        ziapi::http::Context ctx{};

        if (self->m_dtor)
            break;
        {
            std::scoped_lock lck(self->m_req_lock_guard);
            if (!self->m_req.empty()) {
                m_current_req = self->m_req.front();
                self->m_req.erase(self->m_req.begin());
            }
        }
        if (!m_current_req)
            continue;
        res.Bootstrap();
        std::tie(req, ctx) = m_current_req.value();

        for (auto &pre_ref : self->m_mods.pre_processors) {
            auto &pre = pre_ref.get();
            if (pre.ShouldPreProcess(ctx, req))
                pre.PreProcess(ctx, req);
        }

        for (auto &ref_handler : self->m_mods.handlers) {
            auto &handler = ref_handler.get();
            if (handler.ShouldHandle(ctx, req)) {
                handler.Handle(ctx, req, res);
                break;
            }
        }

        for (auto &ref_post : self->m_mods.post_processors) {
            auto &post = ref_post.get();
            if (post.ShouldPostProcess(ctx, req, res))
                post.PostProcess(ctx, req, res);
        }
        {
            std::scoped_lock lck(self->m_res_lock_guard);
            self->m_res.push_back(std::make_pair(res, ctx));
        }
    }
}

}  // namespace zia
