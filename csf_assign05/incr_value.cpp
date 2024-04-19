#include <iostream>
#include <string>
#include "csapp.h"

// Helper function to send a command to the server
void send_command(int fd, const std::string& cmd) {
  std::cout << "Sending: " << cmd;  // Debug output
  if (rio_writen(fd, cmd.c_str(), cmd.length()) < 0) {
    std::cerr << "Communication error on sending command.\n";
    exit(1);
  }
}

// Helper function to read a response from the server
std::string receive_response(int fd) {
  char buf[MAXLINE];
  rio_t rio;
  rio_readinitb(&rio, fd);
  if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
    std::cerr << "Communication error on receiving response.\n";
    exit(1);
  }
  return std::string(buf);
}

int main(int argc, char **argv) {
  if (argc != 6 && (argc != 7 || std::string(argv[1]) != "-t")) {
    std::cerr << "Usage: ./incr_value [-t] <hostname> <port> <username> <table> <key>\n";
    return 1;
  }

  int count = 1;
  bool use_transaction = (argc == 7);
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

  send_command(clientfd, "LOGIN " + username + "\n");
  std::cout << "Response: " << receive_response(clientfd);  // Receive login confirmation

  if (use_transaction) {
    send_command(clientfd, "BEGIN\n");
    std::cout << "Response: " << receive_response(clientfd);  // Begin transaction
  }

  send_command(clientfd, "GET " + table + " " + key + "\n");
  std::string response = receive_response(clientfd);
  std::cout << "Response: " << response;  // Get current value

  // Extract the current value from response assumed to be in the format "DATA value\n"
  int current_value = std::stoi(response.substr(5));  // Assuming "DATA " is 5 chars
  int new_value = current_value + 1;

  send_command(clientfd, "SET " + table + " " + key + " " + std::to_string(new_value) + "\n");
  std::cout << "Response: " << receive_response(clientfd);  // Set new value

  if (use_transaction) {
    send_command(clientfd, "COMMIT\n");
    std::cout << "Response: " << receive_response(clientfd);  // Commit transaction
  }

  send_command(clientfd, "BYE\n");
  std::cout << "Response: " << receive_response(clientfd);  // Close the session

  close(clientfd);
  return 0;
}
