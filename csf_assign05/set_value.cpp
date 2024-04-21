#include <iostream>
#include <cstring> 
#include "csapp.h"
#include "exceptions.h"

void send_message(int fd, const std::string& msg) {
    ssize_t write_result = rio_writen(fd, msg.c_str(), msg.length());
    if (write_result < 0 || static_cast<size_t>(write_result) != msg.length()) {
        throw CommException("Failed to send message: " + msg);
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE); // Clear the buffer to ensure no residual data
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to read response from server.");
    }
    std::string response(buf);
    if (response.empty() or response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated.");
    }
    return response.substr(0, response.length() - 1);  // Remove the newline character
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2]; // Port should be a string for compatibility with open_clientfd
    std::string username = argv[3];
    std::string table = argv[4];
    std::string key = argv[5];
    std::string value = argv[6];

    int clientfd = -1; // Define clientfd here to ensure it is accessible in the catch block

    try {
        clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server at " + hostname + ":" + port);
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw InvalidMessage("Login failed.");
        }

        send_message(clientfd, "PUSH " + value + "\n");
        if (read_response(clientfd, rio) != "OK") {
            throw OperationException("Error pushing value onto stack.");
        }

        send_message(clientfd, "SET " + table + " " + key + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("Failed to set value: " + response);
        } else {
            std::cout << "Value set successfully for " + key + " in " + table + "." << std::endl;
        }

        send_message(clientfd, "BYE\n");
        read_response(clientfd, rio);  // Confirm the BYE command
        close(clientfd);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (clientfd >= 0) {
            send_message(clientfd, "BYE\n"); // Attempt to close connection gracefully
            close(clientfd);
        }
        return 1;
    }
}
