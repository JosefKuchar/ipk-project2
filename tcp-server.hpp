#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#include "args.hpp"
#include "server.hpp"

class TcpServer : public Server {
    using Server::Server;

   public:
    void run();
};

#endif  // __TCP_SERVER_HPP__
