#include <iostream>
#include <cstring>
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) == -1) {
        throw CommException("Failed to send message");
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) == -1) {
        throw CommException("Failed to read response");
    }
    std::string response(buf);
    if (response.find("ERROR") != std::string::npos) {
        throw InvalidMessage("Server returned an error: " + response);
    } else if (response.find("FAILED") != std::string::npos) {
        throw OperationException("Operation failed: " + response);
    }
    return response.substr(0, response.length() - 1); // Remove newline for easier handling
}

int main(int argc, char **argv) {
    if (argc != 6) {
        std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];
    std::string username = argv[3];
    std::string table = argv[4];
    std::string key = argv[5];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        read_response(clientfd, rio); // Expecting "OK"

        send_message(clientfd, "GET " + table + " " + key + "\n");
        send_message(clientfd, "TOP\n");
        std::string response = read_response(clientfd, rio);
        if (response.substr(0, 4) == "DATA") {
            std::cout << response.substr(5) << std::endl; // Print the value
        } else {
            throw OperationException("Expected DATA, received: " + response);
        }

        send_message(clientfd, "BYE\n");
        close(clientfd);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
