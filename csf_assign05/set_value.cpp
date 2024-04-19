#include <iostream>
#include "csapp.h"

// Helper function to send a message to the server
void send_message(int fd, const std::string& msg) {
  if (rio_writen(fd, msg.c_str(), msg.length()) < 0) {
    std::cerr << "Error: Failed to send message: " << msg << std::endl;
    exit(1);
  }
}

// Helper function to read a response from the server
std::string read_response(int fd, rio_t& rio) {
  char buf[MAXLINE];
  if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
    std::cerr << "Error: Failed to read response from server." << std::endl;
    exit(1);
  }
  return std::string(buf);
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

  // Open a client file descriptor to the server
  int clientfd = open_clientfd(hostname.c_str(), port.c_str());
  if (clientfd < 0) {
    std::cerr << "Error: Could not connect to server at " << hostname << ":" << port << std::endl;
    return 1;
  }

  rio_t rio;
  rio_readinitb(&rio, clientfd);

  // Log in to the server
  send_message(clientfd, "LOGIN " + username + "\n");
  if (read_response(clientfd, rio).find("OK") == std::string::npos) {
    std::cerr << "Login failed." << std::endl;
    close(clientfd);
    return 1;
  }

  // Set the value for the key
  send_message(clientfd, "PUSH " + value + "\n");
  if (read_response(clientfd, rio).find("OK") == std::string::npos) {
    std::cerr << "Error pushing value onto stack." << std::endl;
    close(clientfd);
    return 1;
  }

  send_message(clientfd, "SET " + table + " " + key + "\n");
  std::string response = read_response(clientfd, rio);
  if (response.find("OK") != std::string::npos) {
    std::cout << "Value set successfully." << std::endl;
  } else {
    std::cerr << "Failed to set value: " << response << std::endl;
    close(clientfd);
    return 1;
  }

  // Log out from the server
  send_message(clientfd, "BYE\n");
  read_response(clientfd, rio);  // Read final OK from BYE command
  close(clientfd);

  return 0;
}
