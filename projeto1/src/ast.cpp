#include "ast.h"
#include "st.h"

/* Macros that reduce the visual pollution when indentation is needed. */

/*
 * _tab
 *
 * Takes a single line of code and indents it with two spaces.
 */
#define _tab(X)     spaces += 2; (X); spaces -= 2

/*
 * _notab
 *
 * Variadic macro that prevents indentation for any number of lines.
 */
#define _notab(...) int tmp = spaces; spaces = 0; (__VA_ARGS__); spaces = tmp;

using namespace AST;

/* Bison standard error output function. */
extern void yyerror(const char* s, ...);

/* Saves the current indentation status. */
int spaces;

/* Prints an object with `cout`, prepended by `n` spaces. */
template<typename T>
void text(const T& text, int n) {
  std::string blank(n, ' ');
  std::cout << blank << text;
}

void Node::errorMessage(Operation op, Node* n1, Node* n2) {
  yyerror("semantic error: %s operation expected %s but received %s",
    _opt[op].c_str(), _usr[n1->_type()].c_str(), _usr[n2->_type()].c_str());
}

IntNode::IntNode(int value):
value(value) {
  this->type = INT;
}

void IntNode::print(bool prefix) {
  text(value, 1);
}

NodeType IntNode::_type() {
  return this->type;
}

FloatNode::FloatNode(char* value):
value(value) {
  this->type = FLOAT;
}

void FloatNode::print(bool prefix) {
  text(value, 1);
}

NodeType FloatNode::_type() {
  return this->type;
}

BoolNode::BoolNode(bool value):
value(value) {
  this->type = BOOL;
}

void BoolNode::print(bool prefix) {
  text(value ? "true" : "false", 1);
}

NodeType BoolNode::_type() {
  return this->type;
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  if (binOp == index && right->_type() == INT) {
    // only valid index operation
    return;
  } else if (left->_type() % 4 != right->_type() % 4) {
    // the modulo operator ignores the array types, considering them as
    // their primitive types
    if (left->_type() == INT && right->_type() == FLOAT && binOp != assign) {
      // first two ifs ensure coercion
      this->left = new UnaryOpNode(cast_float, left);
    } else if (left->_type() == FLOAT && right->_type() == INT) {
      this->right = new UnaryOpNode(cast_float, right);
    } else if (binOp == index) {
      errorMessage(binOp, new IntNode(0), right);
    } else if (left->_type() != ND) {
      errorMessage(binOp, left, right);
    }
  }
}

void BinaryOpNode::print(bool prefix) {
  if (prefix) {
    text("", binOp != assign);
    text(_bin[binOp], spaces);
    _notab(
      left->print(prefix),
      right->print(prefix));
  } else {
    _notab(
      left->print(!prefix),
      text("", binOp == assign),
      text(_bin[binOp], spaces),
      text("", binOp != assign),
      right->print(!prefix));
  }
}

NodeType BinaryOpNode::_type() {
  return static_cast<int>(binOp) < 6 ? left->_type() : BOOL;
}

UnaryOpNode::UnaryOpNode(Operation op, Node* node):
op(op), node(node) {
  if (op == cast_int) {
    this->type = INT;
  } else if (op == cast_float) {
    this->type = FLOAT;
  } else if (op == cast_bool || op == _not) {
    this->type = BOOL;
  } else if (op == uminus) {
    this->type = node->_type();
  }
}

void UnaryOpNode::print(bool prefix) {
  text(_bin[op], 0);
  node->print(prefix);
}

NodeType UnaryOpNode::_type() {
  return this->type;
}

void VariableNode::print(bool prefix) {
  if (next != nullptr) {
    next->print(false);
    text(",", 0);
  }
  std::string s;
  // prints the size of the array if the `size` attribute is not zero
  s = this->size ? " (size: " + std::to_string(this->size) + ")" : "";
  text(id + s, 1);
}

NodeType VariableNode::_type() {
  return this->type;
}

void BlockNode::print(bool prefix) {
  for (Node* n : nodeList) {
    n->print(prefix);
    if (n->_type() != ND || dynamic_cast<BinaryOpNode*>(n)) {
      // prints new line on assignment to undeclared variable
      text("\n", 0);
    }
  }
}

void MessageNode::print(bool prefix) {
  if (node->_type() != ND) {
    // prints only the primitive type and its type (variable or array)
    text(_var[this->_type() % 4] + this->msg, spaces);
    node->print(false);
  }
}

NodeType MessageNode::_type() {
  return node->_type();
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
condition(condition), _then(_then), _else(_else) {
  // ensures semantic error if condition is not a boolean test
  if (condition->_type() != BOOL) {
    errorMessage(if_test, new AST::BoolNode(0), condition);
  }
}

void IfNode::print(bool prefix) {
  text("if:", spaces);
  _notab(condition->print(true));
  text("\n", 0);
  text("then:\n", spaces);
  _tab(_then->print(true));
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    _tab(_else->print(true));
  }
}

ForNode::ForNode(Node* assign, Node* test, Node* iteration, BlockNode* body):
assign(assign), test(test), iteration(iteration), body(body) {
  if (test->_type() != BOOL) {
    errorMessage(if_test, new AST::BoolNode(0), test);
  }
}

void ForNode::print(bool prefix) {
  text("for: ", spaces);
  _notab(
    assign->print(true),
    text(",", 0),
    test->print(true),
    text(", ", 0),
    iteration->print(true));
  text("\n", 0);
  text("do:\n", spaces);
  _tab(body->print(true));
}
