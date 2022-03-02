#include "http/SafeRequestQueue.hpp"

SafeRequestQueue::SafeRequestQueue(ziapi::http::IRequestOutputQueue &queue) : queue_(queue) {}

void SafeRequestQueue::Push(std::pair<ziapi::http::Request, ziapi::http::Context> &&req)
{
    std::scoped_lock sl(mu_);
    queue_.Push(std::move(req));
}

[[nodiscard]] std::size_t SafeRequestQueue::Size() noexcept
{
    std::scoped_lock sl(mu_);
    return queue_.Size();
}
