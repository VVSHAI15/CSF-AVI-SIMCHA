#include <iostream>
#include <cstring>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) != static_cast<ssize_t>(msg.size())) {
        throw CommException("Failed to send message");
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to read response from server");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated");
    }
    return response.substr(0, response.size() - 1);
}

int main(int argc, char **argv) {
    if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    bool use_transaction = (argc == 7 && std::string(argv[1]) == "-t");
    int index = use_transaction ? 2 : 1;
    std::string hostname = argv[index++];
    std::string port = argv[index++];
    std::string username = argv[index++];
    std::string table = argv[index++];
    std::string key = argv[index++];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw InvalidMessage("Login failed");
        }

        if (use_transaction) {
            send_message(clientfd, "BEGIN\n");
            if (read_response(clientfd, rio) != "OK") {
                throw OperationException("Transaction start failed");
            }
        }

        // Get current value
        send_message(clientfd, "GET " + table + " " + key + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("GET command failed");
        }

        // Increment value
        send_message(clientfd, "PUSH 1\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("PUSH command failed");
        }

        send_message(clientfd, "ADD\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("ADD command failed");
        }

        // Set new value
        send_message(clientfd, "SET " + table + " " + key + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("SET command failed");
        }

        if (use_transaction) {
            send_message(clientfd, "COMMIT\n");
            if (read_response(clientfd, rio) != "OK") {
                throw FailedTransaction("Transaction commit failed");
            }
        }

        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
