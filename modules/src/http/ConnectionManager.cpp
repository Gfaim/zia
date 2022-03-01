#include "http/ConnectionManager.hpp"

void ConnectionManager::Add(SharedConnection conn) { connections_.emplace_back(conn); }

void ConnectionManager::Close(SharedConnection conn)
{
    connections_.erase(std::remove_if(connections_.begin(), connections_.end(),
                                      [conn](const auto &el) { return (el.get() == conn.get()); }));
    conn->Close();
}

void ConnectionManager::CloseAll()
{
    for (auto &conn : connections_) {
        conn->Close();
    }
    connections_.clear();
}
