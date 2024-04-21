#include <iostream>
#include <string>
#include "csapp.h"

// Helper function to send commands to the server and check for OK response
bool send_command_and_check(int fd, const std::string& cmd, rio_t& rio) {
    std::cout << "Sending command: " << cmd;  // For debugging
    if (rio_writen(fd, cmd.c_str(), cmd.length()) < 0) {
        std::cerr << "Communication error on sending command: " << cmd << std::endl;
        return false;
    }
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        std::cerr << "Communication error on receiving response." << std::endl;
        return false;
    }
    std::string response = std::string(buf);
    if (response.substr(0, 2) != "OK") {
        std::cerr << "Server response indicated failure: " << response << std::endl;
        return false;
    }
    return true;
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

    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
        std::cerr << "Failed to connect to the server at " << hostname << ":" << port << std::endl;
        return 1;
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    // Login
    if (!send_command_and_check(clientfd, "LOGIN " + username + "\n", rio)) {
        close(clientfd);
        return 1;
    }

    if (use_transaction && !send_command_and_check(clientfd, "BEGIN\n", rio)) {
        send_command_and_check(clientfd, "BYE\n", rio);
        close(clientfd);
        return 1;
    }

    // Perform the increment sequence
    if (!send_command_and_check(clientfd, "GET " + table + " " + key + "\n", rio) ||
        !send_command_and_check(clientfd, "PUSH 1\n", rio) ||
        !send_command_and_check(clientfd, "ADD\n", rio) ||
        !send_command_and_check(clientfd, "SET " + table + " " + key + "\n", rio)) {
        if (use_transaction) {
            send_command_and_check(clientfd, "ROLLBACK\n", rio);
        }
        send_command_and_check(clientfd, "BYE\n", rio);
        close(clientfd);
        return 1;
    }

    if (use_transaction && !send_command_and_check(clientfd, "COMMIT\n", rio)) {
        send_command_and_check(clientfd, "BYE\n", rio);
        close(clientfd);
        return 1;
    }

    // Logout
    send_command_and_check(clientfd, "BYE\n", rio);
    close(clientfd);
    return 0;
}
