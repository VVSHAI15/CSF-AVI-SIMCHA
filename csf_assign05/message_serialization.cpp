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



void MessageSerialization::decode( const std::string &encoded_msg_, Message &msg )
{

    // Remove the trailing space
   /* if(!encoded_msg.empty())
    {
        encoded_msg.pop_back();
    }*/
   encoded_msg.clear();

   if(encoded_msg.length() > Message::MAX_ENCODED_LEN)
    {
        throw InvalidMessage("Encoded message is too long");
    }

    if(encoded_msg_.empty()||encoded_msg.back != '\n')
    {
        throw InvalidMessage("Encoded message is empty");
    }

    //TODO YOU NEED TO LOOK AT THE UNIT TESTS TO BE ABLE TO GET THIS!!! CHECK HOW THE DECODE WORKS
    if(!msg.is_valid())
    {
        throw InvalidMessage("Decoded message is invalid");
    }
}