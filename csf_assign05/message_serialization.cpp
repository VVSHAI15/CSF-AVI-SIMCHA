#include "message_serialization.h"
#include "exceptions.h"
#include <cassert>
#include <iostream>
#include <map>
#include <sstream>
#include <utility>

void MessageSerialization::encode(const Message &msg,
                                  std::string &encoded_msg) {
  // Start by clearing the encoded_msg
  encoded_msg.clear();

  // Encode the command
  std::stringstream ss;
  ss << msg.message_type_to_string(msg.get_message_type());
  if (msg.is_quoted_text(msg.get_quoted_text())) {
    ss << " ";
    ss << "\"" << msg.get_quoted_text() << "\" ";
  } else if (msg.get_num_args() != 0) {
    ss << " ";
    ss << msg.get_arg(0);
  }

  // additional args
  if (msg.get_message_type() == MessageType::SET ||
      msg.get_message_type() == MessageType::GET) {
    ss << " " << msg.get_arg(1);
  }
  // Newline char
  ss << "\n";

  encoded_msg = ss.str();

  if (encoded_msg.length() > Message::MAX_ENCODED_LEN) {
    throw InvalidMessage("Encoded message is too long");
  }
}

void MessageSerialization::decode(const std::string &encoded_msg,
                                  Message &msg) {
  if (encoded_msg.back() != '\n') {
    throw InvalidMessage("Encoded message must end with a newline.");
  }

  std::istringstream iss(
      encoded_msg.substr(0, encoded_msg.size() - 1)); // Strip the newline
  std::string typeStr;
  iss >> typeStr;

  MessageType type = Message::string_to_message_type(typeStr);
  msg.set_message_type(type);
  msg.clear_args();

  std::string arg;
  char nextChar;
  while (iss >> std::ws && iss.peek() != EOF) {
    nextChar = iss.peek();
    if (nextChar == '"') {
      iss.get();                   // Skip the initial quote
      std::getline(iss, arg, '"'); // Read until the next quote
      if (iss.peek() == ' ' || iss.peek() == EOF) {
        iss.get(); // Skip the space after the quote
      }
      msg.push_arg(arg); // Store without the surrounding quotes
    } else {
      iss >> arg;
      msg.push_arg(arg);
    }
  }

  if (!msg.is_valid()) {
    throw InvalidMessage("Decoded message is not valid.");
  }
}