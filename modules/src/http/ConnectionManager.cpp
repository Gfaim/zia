#include "http/ConnectionManager.hpp"

#include <ziapi/Logger.hpp>

void ConnectionManager::Add(SharedConnection conn)
{
    connections_.emplace_back(conn);
    ziapi::Logger::Debug("new connection established ", conn->RemoteEndpoint());
    conn->Start();
}

void ConnectionManager::Close(SharedConnection conn)
{
    connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                                      [conn](const auto &el) { return (el.get() == conn.get()); }));
    ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
    conn->Close();
}

void ConnectionManager::CloseAll()
{
    for (auto &conn : connections_) {
        ziapi::Logger::Debug("client disconnected ", conn->RemoteEndpoint());
        conn->Close();
    }
    connections_.clear();
}
