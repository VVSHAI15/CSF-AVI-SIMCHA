#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include "csapp.h"
#include "message.h"
#include <set>

class Server; // forward declaration
class Table;  // forward declaration

class ClientConnection {
private:
  Server *m_server;
  int m_client_fd;
  rio_t m_fdbuf;
  std::vector<std::string> locked_tables;
  bool in_transactions;

  // copy constructor and assignment operator are prohibited
  ClientConnection(const ClientConnection &);
  ClientConnection &operator=(const ClientConnection &);

public:
  ClientConnection(Server *server, int client_fd);
  ~ClientConnection();

  void chat_with_client();
  void handle_exceptions(const std::exception &e);
  void commit_transactions();
  void rollback_transactions();
  int get_client_fd() const { return m_client_fd; }

  // TODO: additional member functions
};

#endif // CLIENT_CONNECTION_H
