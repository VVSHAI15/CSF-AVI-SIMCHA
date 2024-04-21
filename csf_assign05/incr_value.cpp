#include <iostream>
#include <string>
#include "csapp.h"

// Helper function to send a command to the server
void send_command(int fd, const std::string& cmd) {
    if (rio_writen(fd, cmd.c_str(), cmd.length()) < 0) {
        std::cerr << "Communication error on sending command: " << cmd << std::endl;
        exit(1);
    }
}

// Helper function to read a response from the server
std::string receive_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        std::cerr << "Communication error on receiving response." << std::endl;
        exit(1);
    }
    return std::string(buf);
}

int main(int argc, char **argv) {
    if (argc != 6 && !(argc == 7 && std::string(argv[1]) == "-t")) {
        std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    int count = 1;
    bool use_transaction = (argc == 7 && std::string(argv[1]) == "-t");
    if (use_transaction) {
        count = 2;  // Adjust index if -t option is used
    }

    std::string hostname = argv[count++];
    std::string port = argv[count++];
    std::string username = argv[count++];
    std::string table = argv[count++];
    std::string key = argv[count++];

    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
        std::cerr << "Failed to connect to the server.\n";
        return 1;
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    // Log in to the server
    send_command(clientfd, "LOGIN " + username + "\n");
    if (receive_response(clientfd, rio) != "OK\n") {
        std::cerr << "Login failed." << std::endl;
        close(clientfd);
        return 1;
    }

    if (use_transaction) {
        send_command(clientfd, "BEGIN\n");
        if (receive_response(clientfd, rio) != "OK\n") {
            std::cerr << "Transaction begin failed." << std::endl;
            close(clientfd);
            return 1;
        }
    }

    send_command(clientfd, "GET " + table + " " + key + "\n");
    std::string response = receive_response(clientfd, rio);
    if (response.substr(0, 5) != "DATA ") {
        std::cerr << "Failed to get current value: " << response << std::endl;
        close(clientfd);
        return 1;
    }
    
    // Extract the current value from response
    int current_value = std::stoi(response.substr(5));
    int new_value = current_value + 1;

    send_command(clientfd, "SET " + table + " " + key + " " + std::to_string(new_value) + "\n");
    if (receive_response(clientfd, rio) != "OK\n") {
        std::cerr << "Failed to set new value." << std::endl;
        close(clientfd);
        return 1;
    }

    if (use_transaction) {
        send_command(clientfd, "COMMIT\n");
        if (receive_response(clientfd, rio) != "OK\n") {
            std::cerr << "Transaction commit failed." << std::endl;
            close(clientfd);
            return 1;
        }
    }

    send_command(clientfd, "BYE\n");
    close(clientfd);
    return 0;
}
