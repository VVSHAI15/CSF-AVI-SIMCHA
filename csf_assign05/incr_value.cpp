#include <iostream>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) != msg.size()) {
        throw CommException("Failed to send command: " + msg);
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to receive server response.");
    }
    return std::string(buf).substr(0, std::strlen(buf) - 1);  // Ensure removal of newline
}

int main(int argc, char **argv) {
    if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    bool use_transaction = argc == 7;
    int index = use_transaction ? 2 : 1;
    std::string hostname = argv[index++], port = argv[index++];
    std::string username = argv[index++], table = argv[index++], key = argv[index];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server.");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw InvalidMessage("Login failed.");
        }

        if (use_transaction) {
            send_message(clientfd, "BEGIN\n");
            if (read_response(clientfd, rio) != "OK") {
                throw FailedTransaction("Transaction start failed.");
            }
        }

        send_message(clientfd, "GET " + table + " " + key + "\n");
        send_message(clientfd, "PUSH 1\n");
        send_message(clientfd, "ADD\n");
        send_message(clientfd, "SET " + table + " " + key + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Increment operation failed.");
        }

        if (use_transaction) {
            send_message(clientfd, "COMMIT\n");
            if (read_response(clientfd, rio) != "OK") {
                throw FailedTransaction("Transaction commit failed.");
            }
        }

        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
