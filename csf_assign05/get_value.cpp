#include <iostream>
#include <cstring>
#include "csapp.h"

void send_message(int fd, const std::string& msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
        std::cerr << "Error: Failed to send message: " << msg << std::endl;
        exit(2);  // Different exit code for send failures
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        std::cerr << "Error: Failed to read response from server." << std::endl;
        exit(3);  // Different exit code for read failures
    }
    return std::string(buf);
}

int main(int argc, char **argv) {
    if (argc != 6) {
        std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
        return 1;
    }

    std::string hostname = argv[1];
    std::string port = argv[2];
    std::string username = argv[3];
    std::string table = argv[4];
    std::string key = argv[5];

    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
        std::cerr << "Error: Could not connect to server at " << hostname << ":" << port << std::endl;
        return 4;  // Different exit code for connection failures
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    send_message(clientfd, "LOGIN " + username + "\n");
    if (read_response(clientfd, rio) != "OK\n") {
        std::cerr << "Login failed." << std::endl;
        close(clientfd);
        return 5;  // Different exit code for login failures
    }

    send_message(clientfd, "GET " + table + " " + key + "\n");
    if (read_response(clientfd, rio) != "OK\n") {
        std::cerr << "Error retrieving key: " << key << std::endl;
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 6;  // Different exit code for GET failures
    }

    send_message(clientfd, "TOP\n");
    std::string response = read_response(clientfd, rio);
    if (response.substr(0, 5) == "DATA ") {
        std::cout << response.substr(5);  // Printing the value correctly
    } else {
        std::cerr << "Failed to retrieve data for key: " << key << "\n" << response << std::endl;
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 7;  // Different exit code for TOP failures
    }

    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 0;
}
