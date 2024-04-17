#include <set>
#include <map>
#include <regex>
#include <cassert>
#include "message.h"
#include "message_serialization.h"  


Message::Message()
  : m_message_type(MessageType::NONE)
{
}

Message::Message( MessageType message_type, std::initializer_list<std::string> args )
  : m_message_type( message_type )
  , m_args( args )
{
}

Message::Message( const Message &other )
  : m_message_type( other.m_message_type )
  , m_args( other.m_args )
{
}

Message::~Message()
{
}

Message &Message::operator=( const Message &rhs )
{
  if (this != &rhs) {
    m_message_type = rhs.m_message_type;
    m_args = rhs.m_args;
  }
  return *this;
}

MessageType Message::get_message_type() const
{
  return m_message_type;
}

void Message::set_message_type(MessageType message_type)
{
  m_message_type = message_type;
}

std::string Message::get_username() const
{
  if (m_args.size() > 0) {
    return m_args[0];
  }
  return "";
}

std::string Message::get_table() const
{
  if (m_args.size() > 0) {
    return m_args[0];
  }
  return "";
}

std::string Message::get_key() const
{
  if (m_args.size() > 1) {
    return m_args[1];
  }
  return "";
}

std::string Message::get_value() const
{
  if (m_args.size() > 0) {
    return m_args[0];
  }
  return "";
}

std::string Message::get_quoted_text() const
{
  if (m_args.size() > 0) {
    return m_args[0];
  }
  return "";
}

void Message::push_arg( const std::string &arg )
{
  m_args.push_back( arg );
}

bool Message::is_quoted_text(const std::string& arg)
{
    return !arg.empty() && arg.front() == '"' && arg.back() == '"';
}

std::string Message::message_type_to_string(MessageType type)
{
    switch(type)
    {
        case MessageType::NONE:
            return "NONE";
        case MessageType::LOGIN:
            return "LOGIN";
        case MessageType::CREATE:
            return "CREATE";
        case MessageType::PUSH:
            return "PUSH";
        case MessageType::POP:
            return "POP";
        case MessageType::TOP:
            return "TOP";
        case MessageType::SET:
            return "SET";
        case MessageType::GET:
            return "GET";
        case MessageType::ADD:
            return "ADD";
        case MessageType::SUB:
            return "SUB";
        case MessageType::DIV:
            return "DIV";
        case MessageType::BEGIN:
            return "BEGIN";
        case MessageType::COMMIT:
            return "COMMIT";
        case MessageType::BYE:
            return "BYE";
        case MessageType::OK:
            return "OK";
        case MessageType::FAILED:
            return "FAILED";
        case MessageType::ERROR:
            return "ERROR";
        case MessageType::DATA:
            return "Data";

        
        default:
            return "";
    }
}


bool Message::is_valid() const
{
  //TODO
  return true;
}
