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
    if (rio_writen(fd, msg.c_str(), msg.size()) !=
        static_cast<ssize_t>(msg.size())) {
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
    int clientfd;

    try {
        clientfd = open_clientfd(hostname.c_str(), port.c_str());
        if (clientfd < 0) {
            throw CommException("Could not connect to server");
        }

        rio_t rio;
        rio_readinitb(&rio, clientfd);

        send_message(clientfd, "LOGIN " + username + "\n");
        std::string login_response = read_response(clientfd, rio);
        if (login_response != "OK") {
            std::string error_message = extractValueBetweenQuotes(login_response);
            if (error_message.empty()) {
                throw InvalidMessage("Login failed");
            } else {
                throw InvalidMessage(error_message);
            }
        }

        if (use_transaction) {
            send_message(clientfd, "BEGIN\n");
            std::string begin_response = read_response(clientfd, rio);
            if (begin_response != "OK") {
                std::string error_message = extractValueBetweenQuotes(begin_response);
                if (error_message.empty()) {
                    throw InvalidMessage("Transaction begin failed");
                } else {
                    throw InvalidMessage(error_message);
                }
            }
        }

        // Increment the value sequence
        send_message(clientfd, "GET " + table + " " + key + "\n");
        std::string get_response = read_response(clientfd, rio);
        if (get_response != "OK") {
            std::string error_message = extractValueBetweenQuotes(get_response);
            if (error_message.empty()) {
                throw OperationException("Failed to get key");
            } else {
                throw OperationException(error_message);
            }
        }

        send_message(clientfd, "PUSH 1\n");
        send_message(clientfd, "ADD\n");
        send_message(clientfd, "SET " + table + " " + key + "\n");
        std::string set_response = read_response(clientfd, rio);
        if (set_response != "OK") {
            std::string error_message = extractValueBetweenQuotes(set_response);
            if (error_message.empty()) {
                throw InvalidMessage("Increment failed");
            } else {
                throw InvalidMessage(error_message);
            }
        }

        if (use_transaction) {
            send_message(clientfd, "COMMIT\n");
            std::string commit_response = read_response(clientfd, rio);
            if (commit_response != "OK") {
                std::string error_message = extractValueBetweenQuotes(commit_response);
                if (error_message.empty()) {
                    throw FailedTransaction("Commit failed");
                } else {
                    throw FailedTransaction(error_message);
                }
            }
        }

        send_message(clientfd, "BYE\n");
        close(clientfd);
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        if (clientfd >= 0) {
            send_message(clientfd, "BYE\n");  // Try to close the connection gracefully
            close(clientfd);
        }
        return 2;
    }
}
