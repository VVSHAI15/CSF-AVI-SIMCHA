#include <iostream>
#include <cstring>
#include "csapp.h"

// Helper function to send a message to the server
void send_message(int fd, const std::string& msg) {
    std::cout << "Sending: " << msg;  // Debug output
    if (rio_writen(fd, msg.c_str(), msg.length()) < 0) {
        std::cerr << "Error: Failed to send message to server." << std::endl;
        exit(2);  // Specific exit code for send failure
    }
}

// Helper function to read a response from the server
std::string read_response(int fd, rio_t& rio) {
    char buf[MAXLINE];
    memset(buf, 0, MAXLINE);  // Clear buffer to avoid reading stale data
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        std::cerr << "Error: Failed to read response from server." << std::endl;
        exit(2);  // Specific exit code for read failure
    }
    std::string response(buf);
    std::cout << "Received: " << response;  // Debug output
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

    // Log in to the server
    send_message(clientfd, "LOGIN " + username + "\n");
    std::string response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Login failed: " << response << std::endl;
        close(clientfd);
        return 1;
    }

    // Push the value onto the stack
    send_message(clientfd, "PUSH " + value + "\n");
    response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Error pushing value: " << response << std::endl;
        send_message(clientfd, "BYE\n");  // Attempt to end session properly
        close(clientfd);
        return 1;
    }

    // Set the value for the key
    send_message(clientfd, "SET " + table + " " + key + "\n");
    response = read_response(clientfd, rio);
    if (response != "OK") {
        std::cerr << "Failed to set value: " << response << std::endl;
        send_message(clientfd, "BYE\n");  // Attempt to end session properly
        close(clientfd);
        return 1;
    } else {
        std::cout << "Value set successfully." << std::endl;
    }

    // Log out from the server
    send_message(clientfd, "BYE\n");
    close(clientfd);

    return 0;
}
