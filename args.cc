#include "args.hpp"
#include <arpa/inet.h>
#include <unistd.h>
#include <charconv>
#include <iostream>

Args::Args(int argc, char** argv) {
    // Parse arguments using getopt
    int option;
    int parsed_port;
    std::string host, port;
    while ((option = getopt(argc, argv, "h:p:m:")) != -1) {
        switch (option) {
            case 'h':
                host = optarg;
                break;
            case 'p':
                port = optarg;
                break;
            case 'm':
                mode = optarg;
                break;
            default:  // Invalid option
                std::cout << "Usage: ipkcpc -h <host> -p <port> -m <mode>" << std::endl;
                exit(0);
        }
    }

    // Check if mode is valid
    if (mode != "tcp" && mode != "udp") {
        std::cerr << "Invalid mode. Please use 'tcp' or 'udp'." << std::endl;
        exit(1);
    }

    // Parse address and check if it is valid
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) <= 0) {
        std::cerr << "Invalid address" << std::endl;
        exit(1);
    }

    // Parse port and check if it is valid
    auto res = std::from_chars(port.data(), port.data() + port.size(), parsed_port);
    if (res.ec != std::errc()) {
        std::cerr << "Invalid port" << std::endl;
        exit(1);
    }

    // Set port and address family
    address.sin_port = htons(parsed_port);
    address.sin_family = AF_INET;
}

Args::Args() {
    // Set default values
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);
    mode = "tcp";
}
