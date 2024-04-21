#include <iostream>
#include <cstring>
#include "csapp.h"

void send_message(int fd, const std::string& msg) {
    std::cout << "Sending: " << msg;  // Debug output to trace what is sent
    if (rio_writen(fd, msg.c_str(), msg.length()) < 0) {
        std::cerr << "Communication error: could not send message to server." << std::endl;
        exit(2);  // Specific exit code for communication errors
    }
}

std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);  // Clear buffer to prevent stale data issues
    if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
        std::cerr << "Communication error: failed to read response from server." << std::endl;
        exit(2);
    }
    std::string response(buf);
    std::cout << "Received: " << response;  // Debug output to trace what is received
    return response;
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
        std::cerr << "Could not connect to server at " << hostname << ":" << port << std::endl;
        return 1;
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    send_message(clientfd, "LOGIN " + username + "\n");
    if (read_response(clientfd, rio).find("OK") == std::string::npos) {
        std::cerr << "Login failed" << std::endl;
        close(clientfd);
        return 1;
    }

    send_message(clientfd, "GET " + table + " " + key + "\n");
    std::string response = read_response(clientfd, rio);
    if (response.find("OK") == std::string::npos) {
        std::cerr << "GET command failed: " << response << std::endl;
        send_message(clientfd, "BYE\n");  // Try to end the session gracefully
        close(clientfd);
        return 1;
    }

    send_message(clientfd, "TOP\n");
    response = read_response(clientfd, rio);
    if (response.substr(0, 4) != "DATA") {
        std::cerr << "Failed to retrieve value: " << response << std::endl;
        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 1;
    } else {
        std::cout << "Value: " << response.substr(5) << std::endl;  // Assume DATA value\n format
    }

    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 0;
}
