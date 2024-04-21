#include "csapp.h"
#include "exceptions.h"
#include <iostream>
#include <cstring>

// Helper function to extract a quoted message from a response
std::string extractQuotedMessage(const std::string& response) {
    size_t firstQuote = response.find('"');
    if (firstQuote == std::string::npos) return "";
    size_t secondQuote = response.find('"', firstQuote + 1);
    if (secondQuote == std::string::npos) return "";
    return response.substr(firstQuote + 1, secondQuote - firstQuote - 1);
}

void send_message(int fd, const std::string &msg) {
  if (rio_writen(fd, msg.c_str(), msg.size()) != static_cast<ssize_t>(msg.size())) {
    throw CommException("Failed to send message");
  }
}

std::string read_response(int fd, rio_t &rio) {
  char buf[MAXLINE];
  if (rio_readlineb(&rio, buf, MAXLINE) <= 0) {
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

  bool transaction = (argc == 7);
  int index = transaction ? 2 : 1;
  std::string hostname = argv[index++];
  std::string port = argv[index++];
  std::string username = argv[index++];
  std::string table = argv[index++];
  std::string key = argv[index++];

  try {
    int clientfd = open_clientfd(hostname.c_str(), port.c_str());
    if (clientfd < 0) {
      throw CommException("Could not connect to server");
    }

    rio_t rio;
    rio_readinitb(&rio, clientfd);

    send_message(clientfd, "LOGIN " + username + "\n");
    std::string rep_login = read_response(clientfd, rio);
    if (rep_login != "OK") {
      std::string error_message = extractQuotedMessage(rep_login);
      if (error_message.empty()) {
        throw InvalidMessage("Failed to login");
      } else {
        throw InvalidMessage(error_message);
      }
    }

    if (transaction) {
      send_message(clientfd, "BEGIN\n");
      std::string rep_begin = read_response(clientfd, rio);
      if (rep_begin != "OK") {
        std::string error_message = extractQuotedMessage(rep_begin);
        if (error_message.empty()) {
          throw OperationException("Failed to begin transaction");
        } else {
          throw OperationException(error_message);
        }
      }
    }

    send_message(clientfd, "GET " + table + " " + key + "\n");
    std::string rep_get = read_response(clientfd, rio);
    if (rep_get != "OK") {
      std::string error_message = extractQuotedMessage(rep_get);
      if (error_message.empty()) {
        throw InvalidMessage("Failed to get value");
      } else {
        throw InvalidMessage(error_message);
      }
    }

    send_message(clientfd, "PUSH 1\n");
    std::string rep_push = read_response(clientfd, rio);
    if (rep_push != "OK") {
      std::string error_message = extractQuotedMessage(rep_push);
      if (error_message.empty()) {
        throw OperationException("Failed to push value");
      } else {
        throw OperationException(error_message);
      }
    }

    send_message(clientfd, "ADD\n");
    std::string rep_add = read_response(clientfd, rio);
    if (rep_add != "OK") {
      std::string error_message = extractQuotedMessage(rep_add);
      if (error_message.empty()) {
        throw OperationException("Failed to add value");
      } else {
        throw OperationException(error_message);
      }
    }

    send_message(clientfd, "SET " + table + " " + key + "\n");
    std::string rep_set = read_response(clientfd, rio);
    if (rep_set != "OK") {
      std::string error_message = extractQuotedMessage(rep_set);
      if (error_message.empty()) {
        throw OperationException("Failed to set value");
      } else {
        throw OperationException(error_message);
      }
    }

    if (transaction) {
      send_message(clientfd, "COMMIT\n");
      std::string rep_commit = read_response(clientfd, rio);
      if (rep_commit != "OK") {
        std::string error_message = extractQuotedMessage(rep_commit);
        if (error_message.empty()) {
          throw OperationException("Failed to commit transaction");
        } else {
          throw OperationException(error_message);
        }
      }
    }

    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 0;
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 2;
  }
}
