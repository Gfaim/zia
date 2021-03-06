#pragma once

#include <memory>
#include <mutex>
#include <ziapi/Logger.hpp>

template <typename TConnection>
class ConnectionManager {
public:
    using SharedConnection = std::shared_ptr<TConnection>;

    ConnectionManager() = default;

    void Add(SharedConnection conn)
    {
        std::scoped_lock sl(mu_);
        connections_.emplace_back(conn);
        ziapi::Logger::Debug("new connection established ", conn->RemoteEndpoint());
        conn->Start();
    }

    void Close(SharedConnection conn)
    {
        std::scoped_lock sl(mu_);
        connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                                          [conn](const auto &el) { return (el.get() == conn.get()); }));
        ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
        conn->Close();
    }

    void CloseAll()
    {
        std::scoped_lock sl(mu_);
        for (auto &conn : connections_) {
            ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
            conn->Close();
        }
        connections_.clear();
    }

    void Dispatch(ziapi::http::IResponseInputQueue::ValueType &res)
    {
        std::scoped_lock sl(mu_);

        // // If content length has not been calculated then set it. (some clients fail when content length is not set.)
        if (res.first.headers.find(ziapi::http::header::kContentLength) == res.first.headers.end()) {
            res.first.headers[ziapi::http::header::kContentLength] = std::to_string(res.first.body.size());
        }
        // If connection header is missing from the response then set it to close.
        if (res.first.headers.find(ziapi::http::header::kConnection) == res.first.headers.end()) {
            res.first.headers[ziapi::http::header::kConnection] = "close";
        }

        for (auto &conn : connections_) {
            auto remote = conn->RemoteEndpoint();
            if (remote.address().to_string() == std::any_cast<std::string>(res.second.at("client.socket.address")) &&
                remote.port() == std::any_cast<std::uint16_t>(res.second.at("client.socket.port"))) {
                if (std::any_cast<std::string>(res.second.at("http.connection")) != "close") {
                    conn->ShouldClose(false);
                }
                conn->Send(res.first);
            }
        }
    }

private:
    std::mutex mu_{};

    std::vector<SharedConnection> connections_{};
};
