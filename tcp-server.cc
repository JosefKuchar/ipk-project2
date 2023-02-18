#include "tcp-server.hpp"
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <regex>
#include <thread>
#include <vector>
#include "parser.hpp"

// Regex patterns for parsing messages
const std::regex re_hello("HELLO\n");
const std::regex re_solve("SOLVE (.*)\n");

void send_message(int client_socket, std::string message) {
    send(client_socket, message.c_str(), message.length(), 0);
}

// This function will be run in a separate thread for each client
void client_handler(int client_socket) {
    Parser parser;
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
            auto result = parser.parse(match[1].str());
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

void signalhandler(int signum) {
    std::cout << "Shutting down server" << std::endl;
    exit(0);
}

void TcpServer::run() {
    std::vector<std::thread> client_threads;
    int server_fd, new_socket;
    int opt = 1;
    int addrlen = sizeof(args.address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
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

    // Start listening for connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Listening on port " << ntohs(args.address.sin_port) << std::endl;

    struct sigaction a;
    a.sa_handler = signalhandler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    // Accept new connections and start a new thread for each client
    while (true) {
        if ((new_socket =
                 accept(server_fd, (struct sockaddr*)&args.address, (socklen_t*)&addrlen)) < 0) {
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
}
