# HTTP Module Documentation

The HTTP module implements the HTTP 1.1 protocol.

## Module Type

The HTTP module implements the following module interfaces:
- `INetworkModule`

## Configuration

This section defines the configuration values the module depends on.

| Key                      | Type   | Required | Description                                                                                                                             |
|--------------------------|--------|----------|-----------------------------------------------------------------------------------------------------------------------------------------|
| `modules.http.enableTls` | `bool` | No       | Dictates whether sockets will be TLS encrypted. It is set to `false` by default.                                                        |
| `modules.http.port`      | `int`  | No       | Determines the port of the TCP socket. By default, it is set to either `80` or `443` depending on the value of `modules.http.enableTls` |

## Context

This section defines the request/response context values the module depends on.

| Key                     | Type            | Actions      | Description                                                                                                 |
|-------------------------|-----------------|--------------|-------------------------------------------------------------------------------------------------------------|
| `client.socket.address` | `std::string`   | Read + Write | The address of the client socket. Used internally to determine which client the response should be sent to. |
| `client.socket.port`    | `std::uint16_t` | Read + Write | The port of the client socket. Used internally to determine which client the response should be sent to.    |
| `http.connection`       | `std::string`   | Read + Write | Used internally to know whether or not to close the TCP connection after sending the response.              |

In addition, the HTTP module may read from and write to `http.*`.

## Additional Comments
