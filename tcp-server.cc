#include "tcp-server.hpp"
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <regex>
#include <thread>
#include <vector>
#include "parser.hpp"

// Regex patterns for parsing messages
const std::regex re_hello("^HELLO\n");
const std::regex re_solve("^SOLVE (.*)\n");

// Server socket
int sock_tcp;
// List of client sockets
std::vector<int> clients;
// Mutex for accessing the list of clients
std::mutex mutex;

/**
 * Send helper function
 * @param client_socket Socket of the client
 * @param message Message to send
 */
void send_message(int client_socket, std::string message) {
    send(client_socket, message.c_str(), message.length(), 0);
}

/**
 * Client handler
 * This function will be run in a separate thread for each client
 * @param client_socket Socket of the client
 */
void client_handler(int client_socket) {
    Parser parser;
    bool hello_received = false;
    std::string msg;

    while (true) {
        // Check if the message contains a newline
        std::size_t pos = msg.find('\n');
        if (pos == std::string::npos) {
            // No newline, continue receiving
            // Receive a message from the client
            char buffer[1024] = {0};
            int valread = read(client_socket, buffer, 1023);
            if (valread == 0) {
                // Client disconnected
                break;
            }
            // Append the buffer to the message
            msg += buffer;
            continue;
        }

        std::smatch match;
        // If we haven't received a HELLO message yet, check if the message is a HELLO message
        if (!hello_received) {
            if (std::regex_search(msg, match, re_hello)) {
                // Remove HELLO from the message
                msg.erase(0, pos + 1);
                // Reply with a HELLO message
                send_message(client_socket, "HELLO\n");
                hello_received = true;
                continue;
            } else {
                // The client didn't send a HELLO message, disconnect
                send_message(client_socket, "BYE\n");
                break;
            }
        }
        /**
         * Only SOLVE messages are allowed after the HELLO message
         * or BYE messages, but we don't need to check for that,
         * because any non-SOLVE message will cause the client to disconnect
         */
        if (std::regex_search(msg, match, re_solve)) {
            auto result = parser.parse(match[1].str());
            if (result.has_value()) {
                // Remove SOLVE from the message
                msg.erase(0, pos + 1);
                // Reply with the result
                send_message(client_socket, "RESULT " + std::to_string(result.value()) + "\n");
            } else {
                // Invalid expression, disconnect
                send_message(client_socket, "BYE\n");
                break;
            }
        } else {
            // BYE or invalid message, disconnect
            send_message(client_socket, "BYE\n");
            break;
        }
    }

    mutex.lock();
    // Remove the client from the list of clients
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    mutex.unlock();

    // Close the socket
    close(client_socket);
}

/**
 * TCP signal handler
 */
void tcp_signalhandler(int signum) {
    // We can't lock the mutex because it could be locked by a client thread
    // Send a BYE message to all clients
    for (auto& client : clients) {
        send_message(client, "BYE\n");
        close(client);
    }
    // Close the server socket
    close(sock_tcp);
    exit(EXIT_SUCCESS);
}

void TcpServer::run() {
    int new_socket;
    int opt = 1;
    int addrlen = sizeof(args.address);

    if ((sock_tcp = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    if (setsockopt(sock_tcp, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // Bind socket to the address and port
    if (bind(sock_tcp, (struct sockaddr*)&args.address, addrlen) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Start listening for connections
    if (listen(sock_tcp, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Set up the signal handler
    struct sigaction a;
    a.sa_handler = tcp_signalhandler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    // Accept new connections and start a new thread for each client
    while (true) {
        // Wait for new connections
        if ((new_socket = accept(sock_tcp, (struct sockaddr*)&args.address, (socklen_t*)&addrlen)) <
            0) {
            break;
        }
        // Add the client to the list of clients
        mutex.lock();
        std::thread client_thread(client_handler, new_socket);
        clients.push_back(new_socket);
        // Detach the thread so it will be destroyed when it finishes
        client_thread.detach();
        mutex.unlock();
    }
}
