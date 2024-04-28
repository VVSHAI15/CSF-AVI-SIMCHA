#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <algorithm>
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
      send_response(MessageType::ERROR, err.what());
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
    if (stack->is_empty())
      throw OperationException("\"Stack empty\"");
    stack->pop();
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
  }
}

void ClientConnection::handle_top() {
  try {
    if (stack->is_empty())
      throw OperationException("\"Stack empty\"");
    std::string topValue = stack->get_top();
    send_response(MessageType::DATA, topValue);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
  }
}

bool ClientConnection::isNumeric(const std::string &str) {
  return std::all_of(str.begin(), str.end(),
                     [](char c) { return std::isdigit(c) || c == '-'; });
}
void ClientConnection::handle_add() {
  try {
    if (stack->is_empty())
      throw OperationException("\"Not enough operands on stack\"");
    std::string num1_str = stack->get_top();
    if (!isNumeric(num1_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop(); // Pop after validation

    if (stack->is_empty())
      throw OperationException(
          "\"Stack underflow on second operand for addition\"");
    std::string num2_str = stack->get_top();
    if (!isNumeric(num2_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop(); // Pop after validation

    int num1 = std::stoi(num1_str);
    int num2 = std::stoi(num2_str);
    int sum = num1 + num2;
    stack->push(std::to_string(sum));
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
  }
}

void ClientConnection::handle_sub() {
  try {
    if (stack->is_empty())
      throw OperationException("\"Not enough operands on stack\"");
    std::string right_str = stack->get_top();
    if (!isNumeric(right_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop();

    if (stack->is_empty())
      throw OperationException(
          "Stack underflow on second operand for subtraction");
    std::string left_str = stack->get_top();
    if (!isNumeric(left_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop();

    int right = std::stoi(right_str);
    int left = std::stoi(left_str);
    int difference = left - right;
    stack->push(std::to_string(difference));
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
  }
}

void ClientConnection::handle_mul() {
  try {
    if (stack->is_empty())
      throw OperationException("\"Not enough operands on stack\"");
    std::string first_str = stack->get_top();
    if (!isNumeric(first_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop();

    if (stack->is_empty())
      throw OperationException(
          "Stack underflow on second operand for multiplication");
    std::string second_str = stack->get_top();
    if (!isNumeric(second_str))
      throw std::invalid_argument("Non-numeric operand");
    stack->pop();

    int first = std::stoi(first_str);
    int second = std::stoi(second_str);
    int product = first * second;
    stack->push(std::to_string(product));
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
  }
}

void ClientConnection::handle_div() {
  try {
    if (stack->is_empty())
      throw OperationException("\"Not enough operands on stack\"");
    std::string divisor_str = stack->get_top();
    if (!isNumeric(divisor_str))
      throw std::invalid_argument(
          "\"Non-numeric operand or division by zero\"");
    if (std::stoi(divisor_str) == 0)
      throw std::invalid_argument("\"Division by zero\"");
    stack->pop();

    if (stack->is_empty())
      throw OperationException(
          "\"Stack underflow on second operand for division\"");
    std::string dividend_str = stack->get_top();
    if (!isNumeric(dividend_str))
      throw std::invalid_argument("\"Non-numeric operand\"");
    stack->pop();

    int divisor = std::stoi(divisor_str);
    int dividend = std::stoi(dividend_str);
    int quotient = dividend / divisor;
    stack->push(std::to_string(quotient));
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    send_response(MessageType::FAILED, e.what());
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


void ClientConnection::handle_begin() {
  if (in_transaction) {
    send_response(MessageType::FAILED, "Transaction already started");
  } else {
    in_transaction = true;
    locked_tables.clear(); // Ensure no tables are marked as locked at the start
    send_response(MessageType::OK);
  }
}

// Handle setting a value in a table
void ClientConnection::handle_set(const Message& message) {
    std::string tableName = message.get_table();
    std::string key = message.get_key();
    std::string value = stack->get_top();  // Assuming value to set is on top of the stack

    Table* table = m_server->find_table(tableName);
    if (!table) {
        send_response(MessageType::ERROR, "Table not found");
        return;
    }

    try {
        if (in_transaction) {
            if (locked_tables.find(tableName) == locked_tables.end()) {
                if (!table->trylock()) {
                    handle_rollback();
                    send_response(MessageType::FAILED, "Failed to lock table, transaction rolled back");
                    return;
                }
                locked_tables.insert(tableName);
            }
        } else {
            table->lock();
        }

        table->set(key, value, in_transaction);
        send_response(MessageType::OK);
        if (!in_transaction) {
            table->unlock();
        }
    } catch (const std::exception& e) {
        if (in_transaction) {
            handle_rollback();
        } else {
            table->unlock();
        }
        send_response(MessageType::FAILED, e.what());
    }
}

// Handle retrieving a value from a table
void ClientConnection::handle_get(const Message& message) {
    std::string tableName = message.get_table();
    std::string key = message.get_key();

    Table* table = m_server->find_table(tableName);
    if (!table) {
        send_response(MessageType::ERROR, "Table not found");
        return;
    }

    try {
        if (in_transaction && locked_tables.find(tableName) == locked_tables.end()) {
            if (!table->trylock()) {
                handle_rollback();
                send_response(MessageType::FAILED, "Failed to lock table, transaction rolled back");
                return;
            }
            locked_tables.insert(tableName);
        } else {
            table->lock();
        }

        std::string value = table->get(key, in_transaction);
        stack->push(value);
        send_response(MessageType::OK);
        if (!in_transaction) {
            table->unlock();
        }
    } catch (const std::exception& e) {
        if (in_transaction) {
            handle_rollback();
        } else {
            table->unlock();
        }
        send_response(MessageType::FAILED, e.what());
    }
}

// Handle committing a transaction
void ClientConnection::handle_commit() {
    if (!in_transaction) {
        send_response(MessageType::FAILED, "No transaction to commit");
        return;
    }

    try {
        for (const auto& tableName : locked_tables) {
            Table* table = m_server->find_table(tableName);
            if (table) {
                table->commit_changes();
                table->unlock();
            }
        }
        locked_tables.clear();
        in_transaction = false;
        send_response(MessageType::OK);
    } catch (const std::exception& e) {
        handle_rollback();
        send_response(MessageType::FAILED, e.what());
    }
}

// Handle rolling back a transaction
void ClientConnection::handle_rollback() {
    for (const auto& tableName : locked_tables) {
        Table* table = m_server->find_table(tableName);
        if (table) {
            table->rollback_changes();
            table->unlock();
        }
    }
    locked_tables.clear();
    in_transaction = false;
    send_response(MessageType::FAILED, "Transaction rolled back");
}


void ClientConnection::send_response(MessageType type,
                                     const std::string &additional_info) {
  Message response(type, {additional_info});
  std::string encoded;
  MessageSerialization::encode(response, encoded);
  rio_writen(m_client_fd, encoded.data(), encoded.size());
}
