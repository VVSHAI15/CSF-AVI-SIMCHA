#include <iostream>
#include <cstring>
#include "csapp.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
        std::cerr << "Error: Failed to send message: " << msg << std::endl;
        exit(2);
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        std::cerr << "Error: Failed to read response from server." << std::endl;
        exit(2);
    }
    std::string response = std::string(buf).substr(0, strlen(buf)-1); // Remove newline
    if (response.empty() || response.find("OK") == std::string::npos) {
        std::cerr << "Server error: " << response << std::endl;
        send_message(fd, "BYE\n");
        close(fd);
        exit(3);
    }
    return response;
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];
    std::string username = argv[3];
    std::string table = argv[4];
    std::string key = argv[5];
    std::string value = argv[6];

    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
        std::cerr << "Error: Could not connect to server at " << hostname << ":" << port << std::endl;
        return 1;
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    send_message(clientfd, "LOGIN " + username + "\n");
    read_response(clientfd, rio); // Validates OK response

    send_message(clientfd, "PUSH " + value + "\n");
    read_response(clientfd, rio); // Validates OK response

    send_message(clientfd, "SET " + table + " " + key + "\n");
    std::string response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Failed to set value, server said: " + response << std::endl;
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 1;
    } else {
        std::cout << "Value set successfully.\n";
    }

    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 0;
}