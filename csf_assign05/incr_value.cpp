#include <iostream>
#include <string>
#include "csapp.h"
#include "exceptions.h"

// Helper function to send a command to the server and check for errors
void send_command(int fd, const std::string& cmd) {
    std::cout << "Sending command: " << cmd;  // Debug output
    if (rio_writen(fd, cmd.c_str(), cmd.size()) == -1) {
        throw CommException("Failed to send command to server.");
    }
}

// Helper function to read a response from the server and handle errors
std::string receive_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) == -1) {
        throw CommException("Failed to receive response from server.");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated with newline.");
    }
    response.pop_back(); // Remove the newline character
    if (response.find("ERROR") != std::string::npos) {
        throw InvalidMessage("Server returned an error: " + response);
    } else if (response.find("FAILED") != std::string::npos) {
        throw OperationException("Operation failed: " + response);
    }
    return response;
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
        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_command(clientfd, "LOGIN " + username + "\n");
        receive_response(clientfd, rio); // Expect "OK"

        if (use_transaction) {
            send_command(clientfd, "BEGIN\n");
            receive_response(clientfd, rio); // Expect "OK"
        }

        send_command(clientfd, "GET " + table + " " + key + "\n");
        receive_response(clientfd, rio); // Should be "OK"

        send_command(clientfd, "PUSH 1\n");
        receive_response(clientfd, rio); // Should be "OK"

        send_command(clientfd, "ADD\n");
        receive_response(clientfd, rio); // Should be "OK"

        send_command(clientfd, "SET " + table + " " + key + "\n");
        receive_response(clientfd, rio); // Should be "OK"

        if (use_transaction) {
            send_command(clientfd, "COMMIT\n");
            receive_response(clientfd, rio); // Expect "OK"
        }

        send_command(clientfd, "BYE\n");
        receive_response(clientfd, rio); // Expect "OK"

        close(clientfd);
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
