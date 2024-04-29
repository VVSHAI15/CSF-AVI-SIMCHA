#include "client_connection.h"
#include "csapp.h"
#include "exceptions.h"
#include "message.h"
#include "message_serialization.h"
#include "server.h"
#include <algorithm>
#include <cassert>
#include <iostream>

// Constructor for ClientConnection: Initializes connection settings and state.
ClientConnection::ClientConnection(Server *server, int client_fd)
    : m_server(server), m_client_fd(client_fd), in_transaction(false),
      stack(new ValueStack()), is_logged_in(false) {
  rio_readinitb(&m_fdbuf, m_client_fd);
}

// Destructor: Ensures that resources are properly released when a
// ClientConnection is destroyed.
ClientConnection::~ClientConnection() {
  Close(m_client_fd);
  delete stack;
}

// Main communication loop handling messages from the client.
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

    // Ensure the user is logged in before proceeding with other commands.
    if (!is_logged_in && message.get_message_type() != MessageType::LOGIN) {
      send_response(MessageType::ERROR, "Must login first");
      break;
    }

    // Handle different types of messages based on their type.
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
      ongoing =
          false; // End the communication loop if "BYE" message is received.
      send_response(MessageType::OK);
      break;
    default:
      send_response(MessageType::ERROR, "Unsupported operation");
      break;
    }
  }
}

// Handles pushing a value onto the client's stack.
void ClientConnection::handle_push(const Message &message) {
  stack->push(message.get_value());
  send_response(MessageType::OK);
}

// Handles popping the top value from the stack and handling exceptions if the
// stack is empty.
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

// Handles retrieving and sending the top value of the stack.
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

// Utility function to check if a string is numeric.
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

// Handles the creation of a new table on the server.
void ClientConnection::handle_create(const Message &message) {
  std::string tableName = message.get_table();
  // Check if the table already exists on the server
  if (m_server->find_table(tableName)) {
    // If the table exists, inform the client of the failure
    send_response(MessageType::FAILED, {"Table already exists"});
  } else {
    // If the table does not exist, create it and confirm creation to the client
    m_server->create_table(tableName);
    send_response(MessageType::OK, {});
  }
}

// Handles setting a value in a specified table, possibly within a transaction
void ClientConnection::handle_set(const Message &message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();

  // Check if there's data on the stack to set
  if (stack->is_empty()) {
    send_response(MessageType::FAILED, "Stack is empty, cannot set value");
    return;
  }

  // Retrieve value from the stack to be set in the table
  std::string value = stack->get_top();
  stack->pop(); // Remove the value from the stack after use

  Table *table = m_server->find_table(tableName);
  // Check if the table exists
  if (!table) {
    send_response(MessageType::ERROR, "Table not found");
    return;
  }

  try {
    // Check transaction status and lock table accordingly
    if (in_transaction) {
      if (!locked_tables.count(tableName)) {
        if (!table->trylock()) {
          send_response(MessageType::FAILED, "Lock failed");
          return;
        }
        locked_tables.insert(tableName);
      }
      // Set value in the table with changes staged for transaction
      table->set(key, value, true);
    } else {
      // Directly modify the table data outside of a transaction
      table->lock();
      table->set(key, value, false);
      table->unlock();
    }
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    // Handle exceptions by sending error messages and rolling back transactions
    // if necessary
    send_response(MessageType::FAILED, e.what());
    if (in_transaction) {
      rollback_transaction();
    }
  }
}

// Retrieves a value from a table and sends it to the client
void ClientConnection::handle_get(const Message &message) {
  std::string tableName = message.get_table();
  std::string key = message.get_key();
  Table *table = m_server->find_table(tableName);
  // Ensure the table exists
  if (!table) {
    send_response(MessageType::ERROR, "Table not found");
    return;
  }

  try {
    // Lock the table, retrieve the value, and then unlock
    table->lock();
    std::string value = table->get(key, in_transaction);
    stack->push(value);
    table->unlock();
    send_response(MessageType::OK, {});
  } catch (const std::exception &e) {
    // If an exception occurs, unlock the table and send an error response
    table->unlock();
    send_response(MessageType::FAILED, e.what());
  }
}

// Begins a new transaction by setting the appropriate flags
void ClientConnection::handle_begin() {
  if (in_transaction) {
    // Send an error if a transaction is already active
    send_response(MessageType::FAILED, "Transaction already started");
  } else {
    // Start a new transaction and clear any previously tracked tables
    in_transaction = true;
    locked_tables.clear();
    send_response(MessageType::OK);
  }
}
// Commits all changes made during the current transaction
void ClientConnection::handle_commit() {
  if (!in_transaction) {
    // If no transaction is active, send an error
    send_response(MessageType::FAILED, "No transaction is active");
    return;
  }

  try {
    // Commit changes for all tables involved in the transaction and unlock them
    for (const auto &tableName : locked_tables) {
      Table *table = m_server->find_table(tableName);
      if (table) {
        table->commit_changes();
        table->unlock();
      }
    }
    // Clear the list of locked tables and mark the transaction as complete
    locked_tables.clear();
    in_transaction = false;
    send_response(MessageType::OK);
  } catch (const std::exception &e) {
    // If committing fails, roll back all changes and inform the client
    rollback_transaction();
    send_response(MessageType::FAILED, e.what());
  }
}

// Rolls back any changes made during the current transaction
void ClientConnection::rollback_transaction() {
  for (const auto &tableName : locked_tables) {
    Table *table = m_server->find_table(tableName);
    if (table) {
      // Discard any staged changes and unlock the table
      table->rollback_changes();
      table->unlock();
    }
  }
  // Clear the list of locked tables and mark the transaction as not active
  locked_tables.clear();
  in_transaction = false;
  // Inform the client that the transaction has been rolled back successfully
  send_response(MessageType::OK);
}
void ClientConnection::send_response(MessageType type,
                                     const std::string &additional_info) {
  Message response(type, {additional_info});
  std::string encoded;
  MessageSerialization::encode(response, encoded);

  ssize_t num_bytes_written =
      rio_writen(m_client_fd, encoded.data(), encoded.size());

  if (num_bytes_written < 0) {
    // Handle the error case where writing fails
    throw CommException("Failed to write to client");
  }

  if (static_cast<size_t>(num_bytes_written) != encoded.size()) {
    // Handle the case where not all bytes were written
    throw CommException("Incomplete write to client");
  }
}