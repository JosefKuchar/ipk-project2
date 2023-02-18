#include "server.hpp"
#include "tcp-server.hpp"
#include "udp-server.hpp"

Server::Server(Args args) {
    this->args = args;
}

Server* Server::create(Args args) {
    if (args.mode == "tcp") {
        return new TcpServer(args);
    } else if (args.mode == "udp") {
        return new UdpServer(args);
    } else {
        throw "Unknown protocol";
    }
}
