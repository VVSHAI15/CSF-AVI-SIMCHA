#include <iostream>
#include <cstring>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
        throw CommException("Failed to send message to server");
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to read response from server");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated with newline");
    }
    return response.substr(0, response.size() - 1);  // Trim the newline
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];
    std::string username = argv[3];
    std::string table = argv[4];
    std::string key = argv[5];
    std::string value = argv[6];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Login failed");
        }

        // Push the value onto the stack
        send_message(clientfd, "PUSH " + value + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Error pushing value onto stack");
        }

        // Set the value for the key
        send_message(clientfd, "SET " + table + " " + key + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("Failed to set value for key: " + key);
        } else {
            std::cout << "Value set successfully." << std::endl;
        }

        // Log out from the server
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 2;
    }
}
