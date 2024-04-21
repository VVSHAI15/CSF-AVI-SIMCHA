#include <iostream>
#include <cstring>
#include "csapp.h"

// Function to send a message to the server and check for errors
void send_message(int fd, const std::string& msg) {
  if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
    std::cerr << "Error: Failed to send message\n";
    exit(1);  // Use specific error codes for different errors if needed
  }
}

// Function to read a response from the server, ensuring it ends with a newline
std::string read_response(int fd, rio_t& rio) {
  char buf[MAXLINE];
  memset(buf, 0, MAXLINE); // Clear the buffer to avoid garbage values
  if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
    std::cerr << "Error: Failed to read server response\n";
    exit(1);
  }
  std::string response(buf);
  if (response.empty() || response.back() != '\n') {
    std::cerr << "Error: Server response not properly terminated\n";
    exit(1);
  }
  return response.substr(0, response.length() - 1);  // Remove the newline character
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
    std::cerr << "Error: Could not connect to server\n";
    return 1;
  }

  rio_t rio;
  rio_readinitb(&rio, clientfd);

  // Login
  send_message(clientfd, "LOGIN " + username + "\n");
  if (read_response(clientfd, rio) != "OK") {
    std::cerr << "Login failed\n";
    close(clientfd);
    return 1;
  }

  // Request the value
  send_message(clientfd, "GET " + table + " " + key + "\n");
  std::string get_response = read_response(clientfd, rio);
  if (get_response != "OK") {
    std::cerr << "Error during GET operation: " << get_response << "\n";
    send_message(clientfd, "BYE\n");
    close(clientfd);
    return 1;
  }

  // Retrieve the top value from the operand stack
  send_message(clientfd, "TOP\n");
  std::string top_response = read_response(clientfd, rio);
  if (top_response.substr(0, 4) == "DATA") {
    std::cout << top_response.substr(5) << std::endl; // Ensure printing the value correctly
  } else {
    std::cerr << "Error retrieving data: " << top_response << "\n";
  }

  // Logout
  send_message(clientfd, "BYE\n");
  close(clientfd);
  return 0;
}
