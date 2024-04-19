#include <iostream>
#include <cstring>
#include "csapp.h" 

// Function to send a message to the server
void send_message(int fd, const std::string& msg) {
  if (rio_writen(fd, msg.c_str(), msg.size()) < 0) {
    std::cerr << "Error: Failed to send message\n";
    exit(1);
  }
}

// Function to read a response from the server
std::string read_response(int fd, rio_t& rio) {
  char buf[MAXLINE];  
  if (rio_readlineb(&rio, buf, MAXLINE) < 0) {
    std::cerr << "Error: Failed to read server response\n";
    exit(1);
  }
  return std::string(buf);
}

int main(int argc, char **argv) {
  if (argc != 6) {
    std::cerr << "Usage: ./get_value <hostname> <port> <username> <table> <key>\n";
    return 1;
  }

  std::string hostname = argv[1];
  std::string port = argv[2]; // Port must be a string
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

  // Send the login message
  send_message(clientfd, "LOGIN " + username + "\n");
  if (read_response(clientfd, rio).find("OK") == std::string::npos) {
    std::cerr << "Login failed\n";
    close(clientfd);
    return 1;
  }

  // Request the value
  send_message(clientfd, "GET " + table + " " + key + "\n");
  send_message(clientfd, "TOP\n");
  std::string response = read_response(clientfd, rio);
  if (response.substr(0, 4) == "DATA") {
    std::cout << response.substr(5); // Print the value
  } else {
    std::cerr << "Error retrieving data\n";
  }

  // Logout
  send_message(clientfd, "BYE\n");
  close(clientfd);
  return 0;
}
