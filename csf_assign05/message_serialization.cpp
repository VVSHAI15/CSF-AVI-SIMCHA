#include <utility>
#include <sstream>
#include <cassert>
#include <map>
#include "exceptions.h"
#include "message_serialization.h"

void MessageSerialization::encode(const Message &msg, std::string &encoded_msg)
{
    // Start by clearing the encoded_msg
    encoded_msg.clear();

    // Encode the command
    std::stringstream ss;
    ss << msg.message_type_to_string(msg.get_message_type());
    if(msg.is_quoted_text(msg.get_quoted_text())) {
      ss << " ";
      ss << "\"" << msg.get_quoted_text() << "\" ";
    } else if(msg.get_num_args() != 0) {
      ss << " ";
      ss << msg.get_arg(0); 
    }

    //additional args
    if (msg.get_message_type() == MessageType::SET || msg.get_message_type() == MessageType::GET) {
      ss << " " << msg.get_arg(1);
    }
    //Newline char
    ss << "\n";

    encoded_msg = ss.str();


  

    if(encoded_msg.length() > Message::MAX_ENCODED_LEN)
    {
        throw InvalidMessage("Encoded message is too long");
    }
}

void MessageSerialization::decode(const std::string &encoded_msg, Message &msg) {
    // Check for maximum length violation
    if (encoded_msg.length() > Message::MAX_ENCODED_LEN) {
        throw InvalidMessage("Encoded message exceeds maximum length.");
    }

    // Ensure the message ends with a newline character
    if (encoded_msg.empty() || encoded_msg.back() != '\n') {
        throw InvalidMessage("Encoded message must end with a newline character.");
    }

    // Remove the newline character for parsing
    std::string trimmed_msg = encoded_msg.substr(0, encoded_msg.length() - 1);

    // Use a stringstream to parse the message components
    std::istringstream iss(trimmed_msg);
    std::string typeStr;
    
    // Extract the message type
    if (!(iss >> typeStr)) {
        throw InvalidMessage("Encoded message is empty or incorrectly formatted.");
    }

    MessageType type = Message::string_to_message_type(typeStr);
    msg.set_message_type(type);  // Set the message type

    // Extract remaining arguments, assuming they're not necessarily quoted
    std::string arg;
    while (iss >> arg) {
        if (Message::is_quoted_text(arg)) {
            // Remove leading and trailing quotes
            arg.erase(0, 1);  // Remove the first character (")
            arg.erase(arg.size() - 1, 1);  // Remove the last character (")
        }
        msg.push_arg(arg);
    }

    // Validate the decoded message
    if (!msg.is_valid()) {
        throw InvalidMessage("Decoded message is invalid.");
    }
}
