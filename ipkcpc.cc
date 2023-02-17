#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>
#include <cstring>
#include <iostream>
#include <regex>
#include <thread>
#include <vector>
#include "parser.hpp"

const std::regex re_hello("HELLO\n");
const std::regex re_solve("SOLVE (.*)\n");

void send_message(int client_socket, std::string message) {
    send(client_socket, message.c_str(), message.length(), 0);
}

// This function will be run in a separate thread for each client
void client_handler(int client_socket) {
    bool hello_received = false;

    while (true) {
        // Receive a message from the client
        char buffer[1024] = {0};
        int valread = read(client_socket, buffer, 1024);
        if (valread == 0) {
            break;
        }

        // Parse the message using regex

        std::string msg = buffer;
        std::smatch match;

        if (!hello_received) {
            if (std::regex_match(msg, match, re_hello)) {
                send_message(client_socket, "HELLO\n");
                hello_received = true;
                continue;
            } else {
                send_message(client_socket, "BYE\n");
                break;
            }
        }

        if (std::regex_match(msg, match, re_solve)) {
            Parser parser(match[1].str());
            auto result = parser.parse();
            if (result.has_value()) {
                send_message(client_socket, "RESULT " + std::to_string(result.value()) + "\n");
            } else {
                send_message(client_socket, "BYE\n");
                break;
            }
        } else {
            send_message(client_socket, "BYE\n");
            break;
        }
    }

    // Close the socket
    close(client_socket);
}

void signal_handler(int signal) {
    std::cout << "Server is shutting down..." << std::endl;

    // TODO: Wait for all threads to finish

    exit(0);
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

    struct sigaction sigint_handler;
    sigint_handler.sa_handler = signal_handler;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;
    sigaction(SIGINT, &sigint_handler, NULL);
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
