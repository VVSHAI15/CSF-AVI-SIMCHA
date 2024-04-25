#include "value_stack.h"
#include "exceptions.h"
#include <stack>

ValueStack::ValueStack()
// TODO: initialize member variable(s) (if necessary)
{}

ValueStack::~ValueStack() {}

bool ValueStack::is_empty() const { return stack.empty(); }

void ValueStack::push(const std::string &value) { stack.push(value); }

std::string ValueStack::get_top() const {
  if ((is_empty())) {
    throw OperationException("Operand Stack is empty");
  }
  return stack.top();
}

void ValueStack::pop() {
  if ((is_empty())) {
    throw OperationException("Operand Stack is empty");
  }
  stack.pop();
}
