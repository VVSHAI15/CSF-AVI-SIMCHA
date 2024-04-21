#include "csapp.h"
#include "exceptions.h"
#include <cstring>
#include <iostream>

// Extracts the value between the first pair of quotes in the input string.
std::string extractValueBetweenQuotes(const std::string &input) {
    size_t start = input.find('"');
    if (start == std::string::npos) {
        return ""; // No opening quote found
    }
    size_t end = input.find('"', start + 1);
    if (end == std::string::npos) {
        return ""; // No closing quote found
    }
    return input.substr(start + 1, end - start - 1);
}

void send_message(int fd, const std::string &msg) {
    if (rio_writen(fd, msg.c_str(), msg.size()) != static_cast<ssize_t>(msg.size())) {
        throw CommException("Failed to send message");
    }
}

std::string read_response(int fd, rio_t &rio) {
    char buf[MAXLINE];
    if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
        throw CommException("Failed to read response from server");
    }
    std::string response(buf);
    if (response.empty() || response.back() != '\n') {
        throw InvalidMessage("Server response not properly terminated");
    }
    return response.substr(0, response.size() - 1);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        std::cerr << "Usage: ./set_value <hostname> <port> <username> <table> <key> <value>\n";
        return 1;
    }

    std::string hostname = argv[1], port = argv[2], username = argv[3],
                table = argv[4], key = argv[5], value = argv[6];
    int clientfd;
    try {
        clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        std::string rep_login = read_response(clientfd, rio);
        if (rep_login != "OK") {
            throw InvalidMessage("Login failed: " + extractValueBetweenQuotes(rep_login));
        }

        // Push the value onto the stack
        send_message(clientfd, "PUSH " + value + "\n");
        std::string rep_push = read_response(clientfd, rio);
        if (rep_push != "OK") {
            throw InvalidMessage("Failed to push value: " + extractValueBetweenQuotes(rep_push));
        }

        // Set the value for the key
        send_message(clientfd, "SET " + table + " " + key + "\n");
        std::string response = read_response(clientfd, rio);
        if (response != "OK") {
            throw InvalidMessage("Failed to set value: " + extractValueBetweenQuotes(response));
        }

        std::cout << "Value set successfully.\n";

        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (clientfd >= 0) {
            send_message(clientfd, "BYE\n");
            close(clientfd);
        }
        return 2;
    }
}
