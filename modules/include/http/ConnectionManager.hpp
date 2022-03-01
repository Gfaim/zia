#pragma once

#include <memory>
#include <utility>

#include "http/Connection.hpp"

class ConnectionManager {
public:
    using SharedConnection = std::shared_ptr<Connection>;

    void Add(SharedConnection conn);

    void Close(SharedConnection conn);

    void CloseAll();

    void Dispatch(const std::pair<ziapi::http::Response, ziapi::http::Context> &res);

private:
    std::mutex mu_;

    std::vector<SharedConnection> connections_;
};
