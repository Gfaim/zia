#pragma once

#include <queue>

#include "ModuleAggregate.hpp"

namespace zia {
class RequestManager {
private:
    zia::ModuleAggregate &m_mods;
    std::vector<std::thread> m_threads;
    std::vector<std::pair<ziapi::http::Request, ziapi::http::Context>> m_req{};
    std::vector<std::pair<ziapi::http::Response, ziapi::http::Context>> m_res{};
    std::mutex m_req_lock_guard{};
    std::mutex m_res_lock_guard{};

public:
    RequestManager(size_t max_threads, zia::ModuleAggregate &mods);
    ~RequestManager();
    void AddRequest(std::pair<ziapi::http::Request, ziapi::http::Context> job);
    std::vector<std::pair<ziapi::http::Response, ziapi::http::Context>> PopResponses();
    void Terminate();

    static void job(RequestManager *self);
};
}  // namespace zia