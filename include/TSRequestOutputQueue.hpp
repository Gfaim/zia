#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <utility>

#include "TSQueue.hpp"
#include "ziapi/Http.hpp"

namespace zia {

class TSRequestOutputQueue : public ziapi::http::IRequestOutputQueue,
                             public TSQueue<std::pair<ziapi::http::Request, ziapi::http::Context>> {
public:
    ~TSRequestOutputQueue() = default;

    std::size_t Size() const noexcept override { return TSQueue::Size(); }

    void Push(std::pair<ziapi::http::Request, ziapi::http::Context> &&req) override
    {
        TSQueue::Push(std::forward<std::pair<ziapi::http::Request, ziapi::http::Context> &&>(req));
    };
};

}  // namespace zia