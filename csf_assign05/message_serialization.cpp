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
    encoded_msg += Message::message_type_to_string(msg.get_message_type()) + " ";

    // Encode the arguments
    for(unsigned i = 0; i < msg.get_num_args(); ++i)
    {
        const auto& arg = msg.get_arg(i);
        if(Message::is_quoted_text(arg)) // Check if it's a quoted text
        {
            encoded_msg += "\"" + arg + "\" ";
        }
        else // It's a regular identifier or value
        {
            encoded_msg += arg + " ";
        }
    }

    // Remove the trailing space
    if(!encoded_msg.empty())
    {
        encoded_msg.pop_back();
    }

    // Add newline character at the end
    encoded_msg += "\n";
}


void MessageSerialization::decode( const std::string &encoded_msg_, Message &msg )
{
  // TODO: implement
}
