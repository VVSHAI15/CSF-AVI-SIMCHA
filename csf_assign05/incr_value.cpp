#include <iostream>
#include <cstdlib>
#include "csapp.h"
#include "exceptions.h"

void send_command(int fd, const std::string& cmd) {
    std::cout << "Sending: " << cmd;  // Debug output
    if (rio_writen(fd, cmd.c_str(), cmd.length()) < 0) {
        throw CommException("Communication error on sending command.");
    }
}

std::string receive_response(int fd) {
    char buf[MAXLINE];
    rio_t rio;
    rio_readinitb(&rio, fd);
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        throw CommException("Communication error on receiving response.");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Response from server not properly formatted.");
    }
    return response.substr(0, response.length() - 1);
}

int main(int argc, char **argv) {
    if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    int count = 1;
    bool use_transaction = (argc == 7);
    if (use_transaction) {
        count = 2;  // Adjust index if -t option is used
    }

    std::string hostname = argv[count++];
    std::string port = argv[count++];
    std::string username = argv[count++];
    std::string table = argv[count++];
    std::string key = argv[count++];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Failed to connect to the server.");
        }

        send_command(clientfd, "LOGIN " + username + "\n");
        if (receive_response(clientfd) != "OK") {
            throw OperationException("Login failed.");
        }

        if (use_transaction) {
            send_command(clientfd, "BEGIN\n");
            if (receive_response(clientfd) != "OK") {
                throw FailedTransaction("Transaction begin failed.");
            }
        }

        send_command(clientfd, "GET " + table + " " + key + "\n");
        std::string response = receive_response(clientfd);
        if (response.substr(0, 4) != "DATA") {
            throw OperationException("GET operation failed.");
        }

        // Extract the current value from response assumed to be in the format "DATA value\n"
        int current_value = std::stoi(response.substr(5));
        int new_value = current_value + 1;

        send_command(clientfd, "SET " + table + " " + key + " " + std::to_string(new_value) + "\n");
        if (receive_response(clientfd) != "OK") {
            throw OperationException("SET operation failed.");
        }

        if (use_transaction) {
            send_command(clientfd, "COMMIT\n");
            if (receive_response(clientfd) != "OK") {
                throw FailedTransaction("Transaction commit failed.");
            }
        }

        send_command(clientfd, "BYE\n");
        close(clientfd);
        std::cout << "Increment successful.\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
