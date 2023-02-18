#include "args.hpp"
#include "server.hpp"

int main(int argc, char* argv[]) {
    // Parse command line arguments
    Args args(argc, argv);
    // Create server
    Server* server = Server::create(args);
    // Run server
    server->run();
    delete server;
    return 0;
}
