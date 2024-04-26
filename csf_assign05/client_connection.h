#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include "csapp.h"
#include "message.h"
#include "server.h"
#include "value_stack.h"
#include <set>
#include <string>

class Server;  // Forward declaration to resolve circular dependency

class ClientConnection {
public:
    ClientConnection(Server *server, int client_fd);
    ~ClientConnection();

    void chat_with_client(); 

private:
    Server *m_server;         // Pointer to server object managing this connection
    int m_client_fd;          // File descriptor for the client socket
    rio_t m_fdbuf;            // Buffered file descriptor info for robust I/O
    std::set<std::string> locked_tables;  // Track tables locked in transaction
    bool in_transaction;      // Flag to check if currently in a transaction
    ValueStack *stack;        // Pointer to the stack used for operations

    // Helper methods for handling different message types
    void handle_login(const Message& message);
    void handle_create(const Message& message);
    void handle_set(const Message& message);
    void handle_get(const Message& message);
    void handle_push(const Message& message);
    void handle_pop();
    void handle_top();
    void handle_add();
    void handle_sub();
    void handle_mul();
    void handle_div();
    void handle_begin();
    void handle_commit();
    void send_response(MessageType type, const std::string& additional_info = "");


};

#endif // CLIENT_CONNECTION_H
