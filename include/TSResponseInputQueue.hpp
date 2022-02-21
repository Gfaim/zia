#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>

#include "TSQueue.hpp"
#include "ziapi/Http.hpp"

namespace zia {

class TSResponseInputQueue : public ziapi::http::IResponseInputQueue,
                             public TSQueue<std::pair<ziapi::http::Response, ziapi::http::Context>> {
public:
    ~TSResponseInputQueue() = default;

    std::size_t Size() const noexcept override { return TSQueue::Size(); }

    std::optional<std::pair<ziapi::http::Response, ziapi::http::Context>> Pop() override { return TSQueue::Pop(); };
};

}  // namespace zia