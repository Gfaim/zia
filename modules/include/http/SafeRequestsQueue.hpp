#include <mutex>

#include "ziapi/Http.hpp"

class SafeRequestQueue {
public:
    SafeRequestQueue(ziapi::http::IRequestOutputQueue &queue);

    void Push(std::pair<ziapi::http::Request, ziapi::http::Context> &&req);

    [[nodiscard]] std::size_t Size() const noexcept;

private:
    std::mutex mu_;

    ziapi::http::IRequestOutputQueue &queue_;
};
