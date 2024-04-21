#include <iostream>
#include <cstring>
#include "csapp.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
        std::cerr << "Error: Failed to send message.\n";
        close(fd);
        exit(1);
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        std::cerr << "Error: Failed to read response from server.\n";
        close(fd);
        exit(1);
    }
    return std::string(buf).erase(buf.find_last_not_of(" \n\r\t")+1);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1], port = argv[2], username = argv[3];
    std::string table = argv[4], key = argv[5], value = argv[6];

    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
        std::cerr << "Error: Could not connect to server.\n";
        return 1;
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    send_message(clientfd, "LOGIN " + username + "\n");
    std::string response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Login failed: " + response << std::endl;
        close(clientfd);
        return 1;
    }

    send_message(clientfd, "PUSH " + value + "\n");
    response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Failed to push value: " + response << std::endl;
        close(clientfd);
        return 1;
    }

    send_message(clientfd, "SET " + table + " " + key + "\n");
    response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Failed to set value: " + response << std::endl;
        close(clientfd);
        return 1;
    } else {
        std::cout << "Value set successfully.\n";
    }

    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 0;
}
