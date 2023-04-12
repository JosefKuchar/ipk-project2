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

// TODO: Should i support multiple commands in one message?

// Regex patterns for parsing messages
const std::regex re_hello("HELLO\n");
const std::regex re_solve("SOLVE (.*)\n");

int server_fd;
std::vector<int> clients;
std::mutex mutex;

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

    mutex.lock();
    // Remove the client from the list of clients
    clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
    mutex.unlock();

    // Close the socket
    close(client_socket);
}

void signalhandler(int signum) {
    // We can't lock the mutex because it could be locked by a client thread
    for (auto& client : clients) {
        send_message(client, "BYE\n");
        close(client);
    }
    // Close the server socket
    close(server_fd);
}

void TcpServer::run() {
    int new_socket;
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

    struct sigaction a;
    a.sa_handler = signalhandler;
    a.sa_flags = 0;
    sigemptyset(&a.sa_mask);
    sigaction(SIGINT, &a, NULL);

    // Accept new connections and start a new thread for each client
    while (true) {
        if ((new_socket =
                 accept(server_fd, (struct sockaddr*)&args.address, (socklen_t*)&addrlen)) < 0) {
            break;
        }

        mutex.lock();
        std::thread client_thread(client_handler, new_socket);
        clients.push_back(new_socket);
        client_thread.detach();  // Detach the thread so it will be destroyed when it finishes
        mutex.unlock();
    }
}
