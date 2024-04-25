#include "server.h"
#include "csapp.h"
#include "exceptions.h"
#include "guard.h"
#include <cassert>
#include <iostream>
#include <memory>

Server::Server() {
  // TODO
}

Server::~Server() {
  if (server_fd >= 0) {
    close(server_fd); // Close server socket
  }

  for (auto &table : tables) {
    delete table;
  }
}

void Server::listen(const std::string &port) {
  // server socket should probably be initated somewhere else
  server_fd = open_listenfd(port.data()); // Open server socket
  if (server_fd < 0) {
    log_error("Error opening server socket\n");
    throw CommException("Error opening server socket\n");
  }
}

/**/
void Server::server_loop() {
  while (true) {
    int client_fd = Accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
      log_error("Error accepting client connection\n");
      throw CommException("Error accepting client connection\n");
    }

    ClientConnection *client = new ClientConnection(this, client_fd);

    // creating a detached thread for the client
    pthread_t thr_id;
    if (pthread_create(&thr_id, NULL, client_worker, client) != 0) {
      delete client; // clean
      throw CommException("Could not create client thread");
    } else {
      pthread_detach(thr_id);
    }
  }

  /**/
}

void *Server::client_worker(void *arg) {
  // TODO: implement

  // Assuming that your ClientConnection class has a member function
  // called chat_with_client(), your implementation might look something
  // like this:
  /*
    std::unique_ptr<ClientConnection> client( static_cast<ClientConnection *>(
    arg ) ); client->chat_with_client(); return nullptr;
  */
}

void Server::log_error(const std::string &what) {
  std::cerr << "Error: " << what << "\n";
}

// TODO: implement member functions
