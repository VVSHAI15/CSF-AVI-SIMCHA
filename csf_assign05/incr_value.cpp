#include <iostream>
#include <cstring>
#include "csapp.h"
#include "exceptions.h"

bool send_command_and_check(int fd, const std::string& cmd, rio_t& rio) {
    std::cout << "Sending: " << cmd;  // Debug output
    if (rio_writen(fd, cmd.c_str(), cmd.length()) < 0) {
        throw CommException("Failed to send command: " + cmd);
    }
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        throw CommException("Failed to read response for command: " + cmd);
    }
    std::string response(buf);
    return response.substr(0, 2) == "OK";
}

int main(int argc, char **argv) {
    if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    bool use_transaction = (argc == 7);
    int count = use_transaction ? 2 : 1;

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

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_command_and_check(clientfd, "LOGIN " + username + "\n", rio);
        if (use_transaction) {
            send_command_and_check(clientfd, "BEGIN\n", rio);
        }

        if (!send_command_and_check(clientfd, "GET " + table + " " + key + "\n", rio) ||
            !send_command_and_check(clientfd, "PUSH 1\n", rio) ||
            !send_command_and_check(clientfd, "ADD\n", rio) ||
            !send_command_and_check(clientfd, "SET " + table + " " + key + "\n", rio)) {
            if (use_transaction) {
                send_command_and_check(clientfd, "ROLLBACK\n", rio);
            }
            throw OperationException("Increment failed during steps.");
        }

        if (use_transaction) {
            send_command_and_check(clientfd, "COMMIT\n", rio);
        }

        send_command_and_check(clientfd, "BYE\n", rio);
        close(clientfd);
    } catch (const std::exception& e) {
        std::cerr << "Error during operation: " << e.what() << std::endl;
        return 2;
    }
    return 0;
}
