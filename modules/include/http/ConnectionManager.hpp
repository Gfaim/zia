#pragma once

#include <memory>

#include "http/Connection.hpp"

class ConnectionManager {
public:
    using SharedConnection = std::shared_ptr<Connection>;

    void Add(SharedConnection conn);

    void Close(SharedConnection conn);

    void CloseAll();

private:
    std::vector<SharedConnection> connections_;
};
