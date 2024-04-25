#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <cassert>
#include <iostream>

ClientConnection::ClientConnection(Server *server, int client_fd)
    : m_server(server), m_client_fd(client_fd) {
  rio_readinitb(&m_fdbuf, m_client_fd);
}

ClientConnection::~ClientConnection() { Close(m_client_fd); }

void ClientConnection::chat_with_client() {
  int counter = 0;

  Message message;
  switch (message.get_message_type()) {
  case MessageType::NONE:
  case MessageType::LOGIN:
  case MessageType::CREATE:
  case MessageType::PUSH:
  case MessageType::POP:
  case MessageType::TOP:
  case MessageType::SET:
  case MessageType::GET:
  case MessageType::ADD:
  case MessageType::SUB:
  case MessageType::MUL:
  case MessageType::DIV:
  case MessageType::BEGIN:
  case MessageType::COMMIT:
  case MessageType::BYE:
  case MessageType::OK:
  case MessageType::FAILED:
  case MessageType::ERROR:
  case MessageType::DATA:
  default:
    break; // TODO
  }
}
