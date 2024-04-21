#include <iostream>
#include <cstring>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
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
    return response.substr(0, response.length() - 1);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    std::string hostname = argv[1], port = argv[2], username = argv[3];
    std::string table = argv[4], key = argv[5];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server at " + hostname + ":" + port);
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw InvalidMessage("Login failed: " + response);
        }

        send_message(clientfd, "GET " + table + " " + key + "\n");
        response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("GET command failed: " + response);
        }

        send_message(clientfd, "TOP\n");
        response = read_response(clientfd, rio);
        if (response.substr(0, 4) == "DATA") {
            std::cout << response.substr(5) << std::endl; // Prints the value
        } else {
            throw OperationException("TOP command failed: " + response);
        }

        send_message(clientfd, "BYE\n");
        read_response(clientfd, rio); // Optionally check for "OK"
        close(clientfd);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
