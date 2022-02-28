#pragma once

#include <atomic>
#include <mutex>
#include <thread>

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
    std::atomic_bool m_dtor{};

public:
    /**
     *  Creates a RequestManager instance
     *
     *  @param max_threads maximum threads allowed to run simultaneously
     *  @param mods the zia::ModuleAggregate class that contains modules
     */
    RequestManager(size_t max_threads, zia::ModuleAggregate &mods);
    ~RequestManager();
    /**
     *  Add a request to the request manager, it will be given to one of available workers, or
     *  will be pending until a worker is available
     *
     *  @param request to be processed
     */
    void AddRequest(std::pair<ziapi::http::Request, ziapi::http::Context> request);
    /**
     *  Pop available responses
     */
    std::vector<std::pair<ziapi::http::Response, ziapi::http::Context>> PopResponses();
    /**
     *  Clear the pending requests
     */
    void Clear();

    static void Worker(RequestManager *self);
};
}  // namespace zia