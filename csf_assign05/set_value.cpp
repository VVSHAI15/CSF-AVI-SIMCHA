#include <iostream>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) != msg.size()) {
        throw CommException("Failed to send message: " + msg);
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        throw CommException("Failed to read response from server.");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated.");
    }
    return response.substr(0, response.length() - 1);  // Remove the newline character
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    try {
        std::string hostname = argv[1], port = argv[2], username = argv[3];
        std::string table = argv[4], key = argv[5], value = argv[6];

        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server at " + hostname + ":" + port);
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Login failed.");
        }

        send_message(clientfd, "PUSH " + value + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Error pushing value onto stack.");
        }

        send_message(clientfd, "SET " + table + " " + key + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("Failed to set value. Server said: " + response);
        }

        send_message(clientfd, "BYE\n");
        read_response(clientfd, rio);  // Ensure we read the final "OK" from BYE

        close(clientfd);
        std::cout << "Value set successfully.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
