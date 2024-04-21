#include <iostream>
#include <cstring>
#include <cstdlib>
#include "csapp.h"
#include "exceptions.h"

// Helper function to send a command to the server and handle errors
void send_command(int fd, const std::string& cmd) {
    if (rio_writen(fd, cmd.c_str(), cmd.size()) < 0) {
        throw CommException("Communication error on sending command: " + cmd);
    }
}

// Helper function to receive a response from the server
std::string receive_response(int fd) {
    rio_t rio;
    rio_readinitb(&rio, fd);
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        throw CommException("Communication error on receiving response.");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated.");
    }
    return response.substr(0, response.length() - 1);
}

int main(int argc, char **argv) {
    if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    bool use_transaction = (argc == 7);
    int index = use_transaction ? 2 : 1;

    std::string hostname = argv[index++];
    std::string port = argv[index++];
    std::string username = argv[index++];
    std::string table = argv[index++];
    std::string key = argv[index++];

    try {
        int clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Failed to connect to the server at " + hostname + ":" + port);
        }

        send_command(clientfd, "LOGIN " + username + "\n");
        if (receive_response(clientfd) != "OK") {
            throw InvalidMessage("Login failed");
        }

        if (use_transaction) {
            send_command(clientfd, "BEGIN\n");
            if (receive_response(clientfd) != "OK") {
                throw OperationException("Failed to start transaction");
            }
        }

        send_command(clientfd, "GET " + table + " " + key + "\n");
        std::string response = receive_response(clientfd);
        if (response.substr(0, 4) != "DATA") {
            throw OperationException("GET command failed: " + response);
        }

        int currentValue = std::stoi(response.substr(5));
        int newValue = currentValue + 1;

        send_command(clientfd, "SET " + table + " " + key + " " + std::to_string(newValue) + "\n");
        if (receive_response(clientfd) != "OK") {
            throw OperationException("SET command failed");
        }

        if (use_transaction) {
            send_command(clientfd, "COMMIT\n");
            if (receive_response(clientfd) != "OK") {
                throw FailedTransaction("Failed to commit transaction");
            }
        }

        send_command(clientfd, "BYE\n");
        receive_response(clientfd);  // Optionally check for "OK"
        close(clientfd);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
