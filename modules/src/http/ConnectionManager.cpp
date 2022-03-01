#include "http/ConnectionManager.hpp"

#include <ziapi/Logger.hpp>

void ConnectionManager::Add(SharedConnection conn)
{
    std::scoped_lock sl(mu_);
    connections_.emplace_back(conn);
    ziapi::Logger::Debug("new connection established ", conn->RemoteEndpoint());
    conn->Start();
}

void ConnectionManager::Close(SharedConnection conn)
{
    std::scoped_lock sl(mu_);
    connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                                      [conn](const auto &el) { return (el.get() == conn.get()); }));
    ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
    conn->Close();
}

void ConnectionManager::CloseAll()
{
    std::scoped_lock sl(mu_);
    for (auto &conn : connections_) {
        ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
        conn->Close();
    }
    connections_.clear();
}

void ConnectionManager::Dispatch(const std::pair<ziapi::http::Response, ziapi::http::Context> &res)
{
    std::scoped_lock sl(mu_);
    for (auto &conn : connections_) {
        auto remote = conn->RemoteEndpoint();
        if (remote.address().to_string() == std::any_cast<std::string>(res.second.at("client.socket.address")) &&
            remote.port() == std::any_cast<std::uint16_t>(res.second.at("client.socket.port"))) {
            conn->Send(res.first);
        }
    }
}
