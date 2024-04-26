#ifndef SERVER_H
#define SERVER_H

#include "client_connection.h"
#include "table.h"
#include <map>
#include <memory>
#include <pthread.h>
#include <string>
#include <vector>

class Server {
private:
  // std::vector<std::shared_ptr<Table>> tables;
  std::vector<std::shared_ptr<Table>> tables;
  int server_socket_fd;

  // copy constructor and assignment operator are prohibited
  Server(const Server &);
  Server &operator=(const Server &);

public:
  Server();
  ~Server();

  void listen(const std::string &port);
  void server_loop();

  static void *client_worker(void *arg);

  void log_error(const std::string &what);

  void create_table(const std::string &name);
  Table *find_table(const std::string &name);
};

#endif // SERVER_H
