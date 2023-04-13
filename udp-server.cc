#include "udp-server.hpp"
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include "parser.hpp"

/**
 * Valid opcodes
 */
enum class Opcode {
    Request = 0,
    Response = 1,
};

/**
 * Valid status codes
 */
enum class Status {
    Ok = 0,
    Error = 1,
};

// Server socket
int sock_udp;

/**
 * UDP signal handler
 */
void udp_signalhandler(int signum) {
    // Close the server socket
    close(sock_udp);
    exit(EXIT_SUCCESS);
}

/**
 * Send helper function
 * @param addr Address of the client
 * @param status Status code
 * @param message Message to send
 */
void send_message(struct sockaddr_in* addr, Status status, std::string message) {
    char buffer[1024];
    buffer[0] = (char)Opcode::Response;
    buffer[1] = (char)status;
    buffer[2] = message.length();
    message.copy(buffer + 3, message.length());

    int len = sizeof(*addr);
    int n = message.length() + 3;
    sendto(sock_udp, buffer, n, MSG_CONFIRM, (const struct sockaddr*)addr, len);
}

void UdpServer::run() {
    Parser parser;
    int new_socket;
    int opt = 1;
    socklen_t len;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(args.address);
    char buffer[1024];

    if ((sock_udp = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(sock_udp, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the address and port
    if (bind(sock_udp, (struct sockaddr*)&args.address, addrlen) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Set up the signal handler
    struct sigaction a;
    a.sa_handler = udp_signalhandler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    // Recieve and send loop
    while (true) {
        // Recieve message
        len = sizeof(client_addr);
        ssize_t n =
            recvfrom(sock_udp, buffer, 1024, MSG_WAITALL, (struct sockaddr*)&client_addr, &len);

        // Only accept requests
        if (buffer[0] != (char)Opcode::Request) {
            send_message(&client_addr, Status::Error, "Invalid opcode");
            continue;
        }

        // Check if the length is valid
        if (n - 2 < (uint8_t)buffer[1] || (uint8_t)buffer[1] == 0) {
            send_message(&client_addr, Status::Error, "Invalid length");
            continue;
        }

        // Parse message and send response
        std::string message(buffer + 2, (uint8_t)buffer[1]);
        auto result = parser.parse(message);
        if (result.has_value()) {
            send_message(&client_addr, Status::Ok, std::to_string(result.value()));
        } else {
            send_message(&client_addr, Status::Error, "Error evaluating expression");
        }
    }

    close(sock_udp);
}
