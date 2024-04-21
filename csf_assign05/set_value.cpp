#include "csapp.h"
#include "exceptions.h"
#include <iostream>
#include <cstring>

std::string extractQuotedText(const std::string& response) {
    std::size_t first = response.find('"');
    std::size_t last = response.rfind('"');
    if (first == std::string::npos || last == std::string::npos || first == last) {
        throw InvalidMessage("Response does not contain properly quoted text.");
    }
    return response.substr(first + 1, last - first - 1);
}

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.length()) != static_cast<ssize_t>(msg.length())) {
        throw CommException("Failed to send message to server.");
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to read response from server.");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated.");
    }
    return response.substr(0, response.length() - 1); // Strip the newline
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1], port = argv[2], username = argv[3], 
                table = argv[4], key = argv[5], value = argv[6];
    int clientfd = -1;
    
    try {
        clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server.");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw InvalidMessage("Login failed: " + extractQuotedText(response));
        }

        // Push the value onto the stack
        send_message(clientfd, "PUSH " + value + "\n");
        response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("Push failed: " + extractQuotedText(response));
        }

        // Set the key to the new value
        send_message(clientfd, "SET " + table + " " + key + "\n");
        response = read_response(clientfd, rio);
        if (response != "OK") {
            throw OperationException("Set failed: " + extractQuotedText(response));
        }

        std::cout << "Value set successfully." << std::endl;

        // Close the connection
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (clientfd >= 0) {
            try {
                send_message(clientfd, "BYE\n");
            } catch (...) {
                // Ignore errors in emergency cleanup
            }
            close(clientfd);
        }
        return 2;
    }
}
