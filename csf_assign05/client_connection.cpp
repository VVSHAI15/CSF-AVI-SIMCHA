#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <cassert>
#include <iostream>

ClientConnection::ClientConnection(Server *server, int client_fd)
    : m_server(server), m_client_fd(client_fd), in_transaction(false),
      stack(new ValueStack()), is_logged_in(false) { // Initialize it as false
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
    try {
      MessageSerialization::decode(buf, message);
    } catch (InvalidMessage &err) {
      send_response(MessageType::FAILED, err.what());
      break;
    }

    if (!is_logged_in && message.get_message_type() != MessageType::LOGIN) {
      send_response(MessageType::ERROR, "Must login first");
      break;
    }

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

void ClientConnection::handle_push(const Message &message) {
  stack->push(message.get_value());
  send_response(MessageType::OK);
}

void ClientConnection::handle_pop() {
  try {
    stack->pop();
    send_response(MessageType::OK);
  } catch (const OperationException& e) {
    send_response(MessageType::ERROR, e.what());
  }
}

void ClientConnection::handle_top() {
  try {
    std::string topValue = stack->get_top();
    send_response(MessageType::DATA, topValue);
  } catch (const OperationException& e) {
    send_response(MessageType::ERROR, e.what());
  }
}

void ClientConnection::handle_add() {
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Addition failed: stack underflow");
    return;
  }
  std::string num1_str = stack->get_top(); stack->pop();
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Addition failed: stack underflow");
    stack->push(num1_str); // Push back the first number to maintain state
    return;
  }
  std::string num2_str = stack->get_top(); stack->pop();

  try {
    int num1 = std::stoi(num1_str);
    int num2 = std::stoi(num2_str);
    int sum = num1 + num2;
    stack->push(std::to_string(sum));
    send_response(MessageType::OK);
  } catch (const std::invalid_argument& e) {
    send_response(MessageType::ERROR, "Addition failed: non-numeric operands");
  }
}

void ClientConnection::handle_sub() {
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Subtraction failed: stack underflow");
    return;
  }
  std::string right_str = stack->get_top(); stack->pop();
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Subtraction failed: stack underflow");
    stack->push(right_str); // Push back the first number to maintain state
    return;
  }
  std::string left_str = stack->get_top(); stack->pop();

  try {
    int right = std::stoi(right_str);
    int left = std::stoi(left_str);
    int difference = left - right;
    stack->push(std::to_string(difference));
    send_response(MessageType::OK);
  } catch (const std::invalid_argument& e) {
    send_response(MessageType::ERROR, "Subtraction failed: non-numeric operands");
  }
}

void ClientConnection::handle_mul() {
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Multiplication failed: stack underflow");
    return;
  }
  std::string first_str = stack->get_top(); stack->pop();
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Multiplication failed: stack underflow");
    stack->push(first_str); // Restore stack state
    return;
  }
  std::string second_str = stack->get_top(); stack->pop();

  try {
    int first = std::stoi(first_str);
    int second = std::stoi(second_str);
    int product = first * second;
    stack->push(std::to_string(product));
    send_response(MessageType::OK);
  } catch (const std::invalid_argument& e) {
    send_response(MessageType::ERROR, "Multiplication failed: non-numeric operands");
  }
}

void ClientConnection::handle_div() {
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Division failed: stack underflow");
    return;
  }
  std::string divisor_str = stack->get_top(); stack->pop();
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Division failed: stack underflow");
    stack->push(divisor_str); // Restore stack state
    return;
  }
  std::string dividend_str = stack->get_top(); stack->pop();

  try {
    int divisor = std::stoi(divisor_str);
    if (divisor == 0) {
      send_response(MessageType::ERROR, "Division by zero");
      return;
    }
    int dividend = std::stoi(dividend_str);
    int quotient = dividend / divisor;
    stack->push(std::to_string(quotient));
    send_response(MessageType::OK);
  } catch (const std::invalid_argument& e) {
    send_response(MessageType::ERROR, "Division failed: non-numeric operands");
  }
}


void ClientConnection::handle_login(const Message &message) {
  if (is_logged_in) {
    send_response(MessageType::ERROR, "Already logged in");
    return;
  }

  is_logged_in = true;
  send_response(MessageType::OK);
}

void ClientConnection::handle_create(const Message &message) {
  std::string tableName = message.get_table();
  if (m_server->find_table(tableName)) {
    send_response(MessageType::FAILED, {"Table already exists"});
  } else {
    m_server->create_table(tableName);
    send_response(MessageType::OK, {});
  }
}

void ClientConnection::handle_set(const Message &message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();

  // Check if the stack has elements before trying to pop
  if (stack->is_empty()) {
    send_response(MessageType::ERROR, "Stack is empty, cannot set value");
    return;
  }

  std::string value =
      stack->get_top(); // Assuming value to set is on top of the stack
  Table *table = m_server->find_table(tableName);
  if (!table) {
    send_response(MessageType::ERROR, "Table not found");
    return;
  }

  if (in_transaction) {
    if (!locked_tables.count(tableName)) {
      if (!table->trylock()) {
        send_response(MessageType::FAILED, "Lock failed");
        return;
      }
      locked_tables.insert(tableName);
    }
  } else {
    table->lock();
  }

  // Proceed to set the value
  table->set(key, value, in_transaction);
  send_response(MessageType::OK, {});

  // Unlock if not in a transaction
  if (!in_transaction) {
    table->unlock();
  }
}

void ClientConnection::handle_get(const Message &message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();
  Table *table = m_server->find_table(tableName);
  if (!table) {
    send_response(MessageType::ERROR, "Table not found");
    return;
  }

  bool shouldUnlock = false; // Flag to determine if we should unlock the table

  // Handle locking based on transaction status
  if (in_transaction) {
    if (!locked_tables.count(tableName)) { // Check if the table is already
                                           // locked in this transaction
      if (!table->trylock()) { // Attempt to lock the table for this transaction
        send_response(MessageType::FAILED,
                      "Lock failed, table is in use by another transaction");
        return;
      }
      locked_tables.insert(
          tableName); // Add to locked tables if lock is successful
    }
  } else {
    table->lock(); // Lock the table for single operation if not in transaction
    shouldUnlock = true; // We need to unlock later
  }

  try {
    std::string value = table->get(key, in_transaction);
    stack->push(value);
    send_response(MessageType::OK, {});
  } catch (const std::exception &e) {
    send_response(MessageType::ERROR, e.what());
  }

  if (shouldUnlock) { // Unlock the table if we locked it here and not in a
                      // transaction
    table->unlock();
  }
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

void ClientConnection::send_response(MessageType type,
                                     const std::string &additional_info) {
  Message response(type, {additional_info});
  std::string encoded;
  MessageSerialization::encode(response, encoded);
  rio_writen(m_client_fd, encoded.data(), encoded.size());
}
