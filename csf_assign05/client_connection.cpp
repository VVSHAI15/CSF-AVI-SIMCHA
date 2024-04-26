#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <iostream>
#include <cassert>

ClientConnection::ClientConnection(Server *server, int client_fd)
    : m_server(server), m_client_fd(client_fd), in_transaction(false), 
      stack(new ValueStack()) {
  rio_readinitb(&m_fdbuf, m_client_fd);
}

ClientConnection::~ClientConnection() {
  Close(m_client_fd);
  delete stack;
}



void ClientConnection::chat_with_client() {
  bool ongoing = true;

  while (ongoing) {
    Message message;
    char buf[MAXLINE]; 
    if (Rio_readlineb(&m_fdbuf, buf, MAXLINE) < 0) {
      throw CommException("Failed to read from client");
    }
    MessageSerialization::decode(buf, message);

    switch (message.get_message_type()) {
      case MessageType::LOGIN:
        handle_login(message);
        break;
      case MessageType::CREATE:
        handle_create(message);
        break;
      case MessageType::SET:
        handle_set(message);
        break;
      case MessageType::GET:
        handle_get(message);
        break;
      case MessageType::PUSH:
        handle_push(message);
        break;
      case MessageType::POP:
        handle_pop();
        break;
      case MessageType::TOP:
        handle_top();
        break;
      case MessageType::ADD:
        handle_add();
        break;
      case MessageType::SUB:
        handle_sub();
        break;
      case MessageType::MUL:
        handle_mul();
        break;
      case MessageType::DIV:
        handle_div();
        break;
      case MessageType::BEGIN:
        handle_begin();
        break;
      case MessageType::COMMIT:
        handle_commit();
        break;
      case MessageType::BYE:
        ongoing = false;
        send_response(MessageType::OK);
        break;
      default:
        send_response(MessageType::ERROR, "Unsupported operation");
        break;
    }
  }
} 


void ClientConnection::handle_push(const Message& message) {
  stack->push(message.get_value());
  send_response(MessageType::OK);
}

void ClientConnection::handle_pop() {
  if (!stack->is_empty()) {
    stack->pop();
    send_response(MessageType::OK);
  } else {
    send_response(MessageType::ERROR, "Stack empty");
  }
}

void ClientConnection::handle_top() {
  if (!stack->is_empty()) {
    send_response(MessageType::DATA, stack->get_top());
  } else {
    send_response(MessageType::ERROR, "Stack empty");
  }
}

void ClientConnection::handle_add() {
    // must check the stack has 2 left 
    int num1 = std::stoi(stack->get_top()); stack->pop();
    int num2 = std::stoi(stack->get_top()); stack->pop();
    int sum = num1 + num2;
    stack->push(std::to_string(sum));
    send_response(MessageType::OK);
}

void ClientConnection::handle_sub() {
  // must check the stack has 2 left 
  int right = std::stoi(stack->get_top()); stack->pop();
  int left = std::stoi(stack->get_top()); stack->pop();
  int difference = left - right;
  stack->push(std::to_string(difference));
  send_response(MessageType::OK);
}

void ClientConnection::handle_mul() {
    // must check the stack has 2 left 
  int first = std::stoi(stack->get_top()); stack->pop();
  int second = std::stoi(stack->get_top()); stack->pop();
  int product = first * second;
  stack->push(std::to_string(product));
  send_response(MessageType::OK);
}

void ClientConnection::handle_div() {
  // we must check the stack has 2 left 
  int divisor = std::stoi(stack->get_top()); stack->pop();
  if (divisor == 0) {
    send_response(MessageType::ERROR, "Division by zero");
    return;
  }
  int dividend = std::stoi(stack->get_top()); stack->pop();
  int quotient = dividend / divisor;
  stack->push(std::to_string(quotient));
  send_response(MessageType::OK);
}

void ClientConnection::handle_login(const Message& message) {
  // TODO 
  send_response(MessageType::OK);
}

void ClientConnection::handle_create(const Message& message) {
  std::string tableName = message.get_table();
  if (m_server->find_table(tableName)) {
    send_response(MessageType::FAILED, {"Table already exists"});
  } else {
    m_server->create_table(tableName);
    send_response(MessageType::OK, {});
  }
}

void ClientConnection::handle_set(const Message& message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();
  std::string value = stack->get_top();  // we are assuming value to set is on top of the stack
  Table* table = m_server->find_table(tableName);
  if (!table) {
    send_response(MessageType::ERROR, {"Table not found"});
    return;
  }
  if (in_transaction && !locked_tables.count(tableName)) {
    if (!table->trylock()) {
      send_response(MessageType::FAILED, {"Lock failed"});
      return;
    }
    locked_tables.insert(tableName);
  } else {
    table->lock();
  }
  table->set(key, value, in_transaction);
  send_response(MessageType::OK, {});
  if (!in_transaction) {
    table->unlock();
  }
}

void ClientConnection::handle_get(const Message& message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();
  Table* table = m_server->find_table(tableName);
  if (!table) {
    send_response(MessageType::ERROR, {"Table not found"});
    return;
  }
  table->lock();
  try {
    std::string value = table->get(key, in_transaction);
    stack->push(value);
    send_response(MessageType::OK, {});
  } catch (const std::exception& e) {
    send_response(MessageType::ERROR, {e.what()});
  }
  table->unlock();
}


void ClientConnection::handle_begin() {
  if (in_transaction) {
    send_response(MessageType::FAILED, {"Transaction already started"});
  } else {
    in_transaction = true;
    send_response(MessageType::OK, {});
  }
}

void ClientConnection::handle_commit() {
  // ToDO Commit transaction
  in_transaction = false;
  send_response(MessageType::OK);
}

void ClientConnection::send_response(MessageType type, const std::string& additional_info = "") {
  Message response(type, {additional_info});
  std::string encoded;
  MessageSerialization::encode(response, encoded);
  rio_writen(m_client_fd, encoded.data(), encoded.size());
}







