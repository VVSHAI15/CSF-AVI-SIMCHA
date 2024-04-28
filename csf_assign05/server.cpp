#include "server.h"
#include "csapp.h"
#include "exceptions.h"
#include "guard.h"
#include <cassert>
#include <iostream>
#include <memory>

Server::Server() {
  server_socket_fd = socket(AF_INET, SOCK_STREAM, 0); // server socket
  if (server_socket_fd < 0) {
    log_error("Error creating server socket\n");
  }
}

Server::~Server() {
  if (server_socket_fd >= 0) {
    close(server_socket_fd); // Close server socket
  }

  // Table closing handeled in table deconstructor.
}

void Server::listen(const std::string &port) {
  server_socket_fd = open_listenfd(
      port.c_str()); // Open server socket and establish client connection
  if (server_socket_fd < 0) {
    log_error("Error opening server socket\n");
  }
}

/**/
void Server::server_loop() {
  while (1) {
    int client_fd = Accept(server_socket_fd, NULL, NULL);
    if (client_fd < 0) {
      log_error("Error accepting client connection\n");
      continue; // Doesn't create client connection
    }
    ClientConnection *client = new ClientConnection(this, client_fd);

    // creating a detached thread for the client to allow for concurency and >1
    // connections
    pthread_t thr_id;
    if (pthread_create(&thr_id, NULL, client_worker, client) != 0) {
      close(client_fd); // clean
      log_error("Could not create client thread");
    }
  }
}

void *Server::client_worker(void *arg) {
  pthread_detach(pthread_self()); // we want to develop client seperation so
                                  // we detatch the thread
  std::unique_ptr<ClientConnection> client(
      static_cast<ClientConnection *>(arg));
  try {
    client->chat_with_client();
  } catch (CommException &ex) { // Just in case of a communication error
    // client->handle_exceptions(ex); //TODO figure this out!!!!!
  }
  close(client->get_client_fd());
  return nullptr;
}

void Server::log_error(const std::string &what) {
  std::cerr << "Error: " << what << "\n";
}

void Server::create_table(const std::string &name) {
  std::shared_ptr<Table> table = std::make_shared<Table>(name);
  tables.push_back(table);
}

Table *Server::find_table(const std::string &name) {
  for (std::shared_ptr<Table> table : tables) {
    if (table->get_name() == name) {
      return table.get();
    }
  }
  return nullptr;
}