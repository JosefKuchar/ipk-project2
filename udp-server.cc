#include "udp-server.hpp"
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "parser.hpp"

enum class Opcode {
    Request = 0,
    Response = 1,
};

enum class Status {
    Ok = 0,
    Error = 1,
};

void send_message(int fd, struct sockaddr_in* addr, Status status, std::string message) {
    char buffer[1024];
    buffer[0] = (char)Opcode::Response;
    buffer[1] = (char)status;
    buffer[2] = message.length();
    message.copy(buffer + 3, message.length());

    int len = sizeof(*addr);
    int n = message.length() + 3;
    sendto(fd, buffer, n, MSG_CONFIRM, (const struct sockaddr*)addr, len);
}

void UdpServer::run() {
    int server_fd, new_socket;
    int opt = 1;
    socklen_t len;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(args.address);
    char buffer[1024];

    if ((server_fd = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the address and port
    if (bind(server_fd, (struct sockaddr*)&args.address, addrlen) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << ntohs(args.address.sin_port) << std::endl;

    while (true) {
        len = sizeof(client_addr);
        ssize_t n =
            recvfrom(server_fd, buffer, 1024, MSG_WAITALL, (struct sockaddr*)&client_addr, &len);

        // Only accept requests
        if (buffer[0] != (char)Opcode::Request) {
            send_message(server_fd, &client_addr, Status::Error, "Invalid opcode");
            continue;
        }

        // Check if the length is valid
        if (n - 2 < (int)buffer[1] || (int)buffer[1] == 0) {
            send_message(server_fd, &client_addr, Status::Error, "Invalid length");
            continue;
        }

        // Parse message and send response
        std::string message(buffer + 2, n - 2);
        Parser parser(message);
        auto result = parser.parse();
        if (result.has_value()) {
            send_message(server_fd, &client_addr, Status::Ok, std::to_string(result.value()));
        } else {
            send_message(server_fd, &client_addr, Status::Error, "Error parsing expression");
        }
    }

    close(server_fd);
}
