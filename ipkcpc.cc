#include <iostream>
#include "args.hpp"
#include "server.hpp"

void signal_handler(int signal) {
    std::cout << "Server is shutting down..." << std::endl;

    // TODO: Wait for all threads to finish

    exit(0);
}

int main(int argc, char* argv[]) {
    Args args(argc, argv);
    Server* server = Server::create(args);
    server->run();
    delete server;

    // Create socket file descriptor
    // int type = mode == "tcp" ? SOCK_STREAM : SOCK_DGRAM;

    // struct sigaction sigint_handler;
    // sigint_handler.sa_handler = signal_handler;
    // sigemptyset(&sigint_handler.sa_mask);
    // sigint_handler.sa_flags = 0;
    // sigaction(SIGINT, &sigint_handler, NULL);
    // std::cout << "Server is listening for connections on port " + port << std::endl;

    return 0;
}
