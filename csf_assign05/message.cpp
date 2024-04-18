#include <set>
#include <map>
#include <regex>
#include <cassert>
#include "message.h"
#include "message_serialization.h"  
#include <limits>


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


//For Encoding
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
            return "DATA";

        
        default:
            return "";
    }
}

//For Decoding
MessageType Message::string_to_message_type(const std::string& typeStr) {
    if (typeStr == "NONE") {
        return MessageType::NONE;
    } else if (typeStr == "LOGIN") {
        return MessageType::LOGIN;
    } else if (typeStr == "CREATE") {
        return MessageType::CREATE;
    } else if (typeStr == "PUSH") {
        return MessageType::PUSH;
    } else if (typeStr == "POP") {
        return MessageType::POP;
    } else if (typeStr == "TOP") {
        return MessageType::TOP;
    } else if (typeStr == "SET") {
        return MessageType::SET;
    } else if (typeStr == "GET") {
        return MessageType::GET;
    } else if (typeStr == "ADD") {
        return MessageType::ADD;
    } else if (typeStr == "SUB") {
        return MessageType::SUB;
    } else if (typeStr == "MUL") {
        return MessageType::MUL;
    } else if (typeStr == "DIV") {
        return MessageType::DIV;
    } else if (typeStr == "BEGIN") {
        return MessageType::BEGIN;
    } else if (typeStr == "COMMIT") {
        return MessageType::COMMIT;
    } else if (typeStr == "BYE") {
        return MessageType::BYE;
    } else if (typeStr == "OK") {
        return MessageType::OK;
    } else if (typeStr == "FAILED") {
        return MessageType::FAILED;
    } else if (typeStr == "ERROR") {
        return MessageType::ERROR;
    } else if (typeStr == "DATA") {
        return MessageType::DATA;
    } else {
        return MessageType::NONE; // Default case if no matches
    }

}


bool Message::no_args() const 
{
    return get_num_args() == 0;
}

bool Message::checkIdentifier(const std::string& arg) const
{
    char first_char = arg[0];
    if(!std::isalpha(first_char)) {
        return false;
    }
    
    int char_length = arg.size();
    for(int i = 1; i < char_length; i++) {
        if(!std::isalnum(arg[i]) && arg[i] != '_') {
            return false;
        }
    }
    return true;
}

bool Message::checkValue(const std::string& arg) const
{
    int char_length = arg.size();
    for(int i = 0; i < char_length; i++) {
        if(std::isspace(arg[i])) {
            return false;
        }
    }
    return true;
}

bool Message::is_valid() const
{
   switch (get_message_type()) {
        case MessageType::NONE:
            return no_args();

        case MessageType::LOGIN: 
        case MessageType::CREATE: 
            return m_args.size() == 1 && checkIdentifier(m_args.at(0));

        case MessageType::SET: 
        case MessageType::GET: 
            return m_args.size() == 2 && checkIdentifier(m_args.at(0)) && checkIdentifier(m_args.at(1));

        case MessageType::PUSH: 
        case MessageType::DATA: 
            return m_args.size() == 1 && checkValue(m_args.at(0));

        case MessageType::POP:
        case MessageType::TOP:
        case MessageType::ADD:
        case MessageType::SUB:
        case MessageType::MUL:
        case MessageType::DIV:
        case MessageType::BEGIN:
        case MessageType::COMMIT:
        case MessageType::BYE:
        case MessageType::OK:
            return no_args();

        case MessageType::FAILED: 
        case MessageType::ERROR: 
            return m_args.size() == 1;

        default: 
            return false;
    }
}

