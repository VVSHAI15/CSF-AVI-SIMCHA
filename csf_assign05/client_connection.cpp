#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <cassert>
#include <iostream>
#include <sstream>
#include <vector>

ClientConnection::ClientConnection(Server *server, int client_fd)
    : m_server(server), m_client_fd(client_fd) {
  rio_readinitb(&m_fdbuf, m_client_fd);
}

ClientConnection::~ClientConnection() { close(m_client_fd); }

void ClientConnection::chat_with_client() {
  bool ongoing = true;
  ValueStack operand_stack;
  std::vector<std::string> modified_tables;
  bool transaction = false;

  while (ongoing) {
    try {
      Message message = receive_and_decode_message();
      handle_message(message, operand_stack, modified_tables, transaction,
                     ongoing);
    } catch (const std::exception &ex) {
      handle_exception(ex, modified_tables, transaction);
      ongoing = false;
    }
  }
}

Message ClientConnection::receive_and_decode_message() {
  char buf[2048];
  rio_readlineb(&m_fdbuf, buf, sizeof(buf));
  Message message;
  MessageSerialization::decode(buf,
                               message); // Potentially throws InvalidMessage
  return message;
}

void ClientConnection::handle_message(Message &message,
                                      ValueStack &operand_stack,
                                      std::vector<std::string> &modified_tables,
                                      bool &transaction, bool &ongoing) {
  switch (message.get_message_type()) {
  case MessageType::LOGIN:
  case MessageType::BEGIN:
  case MessageType::COMMIT:
  case MessageType::BYE:
    process_control_commands(message, modified_tables, transaction, ongoing);
    break;
  case MessageType::CREATE:
  case MessageType::PUSH:
  case MessageType::POP:
  case MessageType::TOP:
  case MessageType::SET:
  case MessageType::GET:
  case MessageType::ADD:
  case MessageType::MUL:
  case MessageType::SUB:
  case MessageType::DIV:
    process_data_commands(message, operand_stack, modified_tables, transaction);
    break;
  default:
    throw InvalidMessage("Unsupported command type received");
  }
}

void ClientConnection::process_control_commands(
    Message &message, std::vector<std::string> &modified_tables,
    bool &transaction, bool &ongoing) {
  if (message.get_message_type() == MessageType::BYE) {
    send_response("OK");
    ongoing = false;
  } else if (message.get_message_type() == MessageType::BEGIN) {
    if (transaction) {
      throw FailedTransaction("Transaction already started");
    }
    transaction = true;
    send_response("OK");
  } else if (message.get_message_type() == MessageType::COMMIT) {
    if (!transaction) {
      throw FailedTransaction("No transaction started");
    }
    commit_transaction(modified_tables);
    transaction = false;
    send_response("OK");
  } else {
    send_response("OK");
  }
}

void ClientConnection::process_data_commands(
    Message &message, ValueStack &operand_stack,
    std::vector<std::string> &modified_tables, bool &transaction) {
  // Example implementation for SET command
  if (message.get_message_type() == MessageType::SET) {
    std::string table_name = message.get_table();
    std::string key = message.get_key();
    std::string value =
        operand_stack
            .pop(); // Assumes the value to set is always on top of the stack
    Table *table = m_server->getTable(table_name);
    if (!table) {
      throw InvalidMessage("Table does not exist");
    }
    if (transaction) {
      if (std::find(modified_tables.begin(), modified_tables.end(),
                    table_name) == modified_tables.end()) {
        if (!table->trylock()) {
          throw FailedTransaction("Failed to lock table");
        }
        modified_tables.push_back(table_name);
      }
    } else {
      table->lock();
    }
    table->set(key, value, transaction);
    if (!transaction) {
      table->unlock();
    }
    send_response("OK");
  }
  // Add logic for other data commands as needed
}

void ClientConnection::commit_transaction(
    std::vector<std::string> &modified_tables) {
  for (const auto &tableName : modified_tables) {
    Table *table = m_server->getTable(tableName);
    if (table) {
      table->commit_changes();
      table->unlock();
    }
  }
  modified_tables.clear();
}

void ClientConnection::handle_exception(
    const std::exception &ex, std::vector<std::string> &modified_tables,
    bool &transaction) {
  if (transaction) {
    rollback_transaction(modified_tables);
    transaction = false;
  }
  send_error(ex.what());
}

void ClientConnection::rollback_transaction(
    std::vector<std::string> &modified_tables) {
  for (const auto &tableName : modified_tables) {
    Table *table = m_server->getTable(tableName);
    if (table) {
      table->rollback_changes();
      table->unlock();
    }
  }
  modified_tables.clear();
}

void ClientConnection::send_response(const std::string &response) {
  std::string full_response = response + "\n";
  rio_writen(m_client_fd, full_response.c_str(), full_response.size());
}

void ClientConnection::send_error(const std::string &error_message) {
  std::string full_error = "ERROR " + error_message + "\n";
  rio_writen(m_client_fd, full_error.c_str(), full_error.size());
}
