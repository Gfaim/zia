#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>
#include <thread>

namespace zia {

template <typename T>
class TSQueue {
public:
    virtual ~TSQueue() = default;

    const T& Front()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.front();
    }

    std::optional<T> Pop()
    {
        if (!Size())
            return std::nullopt;
        std::scoped_lock lock(muxQueue);
        auto t = std::move(deqQueue.front());
        deqQueue.pop_front();
        return t;
    }

    void Push(T&& item)
    {
        std::scoped_lock lock(muxQueue);
        deqQueue.emplace_back(std::move(item));

        std::unique_lock<std::mutex> ul(muxBlocking);
        cvBlocking.notify_one();
    }

    bool Empty()
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.empty();
    }

    size_t Size() const
    {
        std::scoped_lock lock(muxQueue);
        return deqQueue.size();
    }

    void Clear()
    {
        std::scoped_lock lock(muxQueue);
        deqQueue.clear();
    }

    void Wait()
    {
        while (Empty()) {
            std::unique_lock<std::mutex> ul(muxBlocking);
            cvBlocking.wait(ul);
        }
    }

protected:
    mutable std::mutex muxQueue{};
    std::deque<T> deqQueue{};
    std::condition_variable cvBlocking{};
    std::mutex muxBlocking{};
};

}  // namespace zia
