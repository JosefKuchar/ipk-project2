#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>
#include <regex>
#include <thread>
#include <vector>
#include "parser.hpp"

// This function will be run in a separate thread for each client
void client_handler(int client_socket) {
    char buffer[1024] = {0};

    // Receive a message from the client
    int valread = read(client_socket, buffer, 1024);
    std::cout << "Received message from the client: " << buffer << std::endl;

    Parser parser(buffer);
    auto result = parser.parse();
    if (result.has_value()) {
        std::string message = std::to_string(result.value());
        send(client_socket, message.c_str(), message.length(), 0);
        std::cout << "Message sent to the client." << std::endl;
    } else {
        std::cout << "Error" << std::endl;
    }

    // Close the socket
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    std::vector<std::thread> client_threads;

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    // Bind socket to the address and port
    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening for connections on port 8080." << std::endl;

    // Accept new connections and start a new thread for each client
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen)) <
            0) {
            perror("Accept failed");
            exit(EXIT_FAILURE);
        }

        std::thread client_thread(client_handler, new_socket);
        client_threads.push_back(std::move(client_thread));
    }

    // Join all threads before exiting
    for (auto& thread : client_threads) {
        thread.join();
    }

    return 0;
}
