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

std::string Node::verboseType() {
  if (this->_type() < 0) {
    return "undefined";
  }
  std::string basic = _usr[this->_type() % 3], ptr = "";
  for (int i = 0; i < this->ptr_cnt; i++) {
    ptr += " pointer";
  }
  std::string is_array = (this->_type() % 6 > 2) ? " array" : "";
  return basic + ptr + is_array;
}

void Node::errorMessage(Operation op, Node* n1, Node* n2) {
  yyerror("semantic error: %s operation expected %s but received %s",
    _opt[op].c_str(), n1->verboseType().c_str(), n2->verboseType().c_str());
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

FloatNode::~FloatNode() {
  free(value);
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
  } else if (binOp == ref && (left->_type() < 0 || right->_type() < 0)) {
    yyerror("semantic error: reference operation expects a pointer");
  } else if (left->_type() != right->_type()) {
    // first two ifs ensure coercion
    if (left->_type() == INT && right->_type() == FLOAT && binOp != assign) {
      this->left = new UnaryOpNode(cast_float, left);
    } else if (left->_type() == FLOAT && right->_type() == INT) {
      this->right = new UnaryOpNode(cast_float, right);
    } else if (binOp == index) {
      AST::Node* n = new IntNode(0);
      errorMessage(binOp, n, right);
      delete n;
    } else if (left->_type() != ND) {
      errorMessage(binOp, left, right);
    }
  }
}

void BinaryOpNode::print(bool prefix) {
  if (prefix) {
    // prints a space if the operation is not an assignment
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
  if (binOp == index) {
    // needs to return primitive type of element inside array
    return static_cast<NodeType>(static_cast<int>(left->_type()) - 3);
  }
  return (binOp < 8) ? left->_type() : BOOL;
}

BinaryOpNode::~BinaryOpNode() {
  delete left;
  delete right;
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
  } else if (op == ref) {
    this->type = static_cast<NodeType>(static_cast<int>(node->_type()) - 6);
  } else if (op == addr) {
    this->type = static_cast<NodeType>(static_cast<int>(node->_type()) + 6);
    if (!dynamic_cast<VariableNode*>(node)) {
      AST::UnaryOpNode* test = dynamic_cast<UnaryOpNode*>(node);
      if (test != nullptr && test->op != index) {
        yyerror(
          "semantic error: address operation expects a variable or array item");
      }
    }
  }
}

void UnaryOpNode::print(bool prefix) {
  text(_bin[op], 0);
  node->print(prefix);
}

NodeType UnaryOpNode::_type() {
  return this->type;
}

UnaryOpNode::~UnaryOpNode() {
  delete node;
}

VariableNode::VariableNode(char* id, Node* next, NodeType type, int size,
                           int ref):
id(id), next(next), type(type), size(size) {
  this->ptr_cnt = ref;
  if (this->type < 3) {
    // since enums are integers with names, we shift them by n positions
    // depending on their status as arrays and/or pointers
    // if they don't have primitive types, then this process is not necessary
    int s = (this->size) ? 3 : 0;
    s += 6 * this->ptr_cnt;
    this->type = static_cast<AST::NodeType>(static_cast<int>(this->type) + s);
  }
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

VariableNode::~VariableNode() {
  free(id);
  delete next;
}

void BlockNode::print(bool prefix) {
  for (Node* n : nodeList) {
    n->print(prefix);
    if (!dynamic_cast<IfNode*>(n) && !dynamic_cast<ForNode*>(n)) {
      // don't print an extra new line after these nodes
      text("\n", 0);
    }
  }
}

BlockNode::~BlockNode() {
  for (Node* n : nodeList) {
    delete n;
  }
}

MessageNode::MessageNode(Node* node, std::string msg, int ref):
node(node), msg(msg) {
  this->ptr_cnt = ref;
}

void MessageNode::print(bool prefix) {
  if (node->_type() != ND) {
    for (int i = 0; i < this->ptr_cnt; i++) {
      this->msg = " ref" + this->msg;
    }
    // prints only the primitive type and its type (variable or array)
    text(_var[this->_type() % 3] + this->msg, spaces);
    node->print(false);
  }
}

NodeType MessageNode::_type() {
  return node->_type();
}

MessageNode::~MessageNode() {
  delete node;
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
condition(condition), _then(_then), _else(_else) {
  // ensures semantic error if condition is not a boolean test
  if (condition->_type() != BOOL) {
    AST::Node* _bool = new AST::BoolNode(0);
    errorMessage(if_test, _bool, condition);
    delete _bool;
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

IfNode::~IfNode() {
  delete condition;
  delete _then;
  delete _else;
}

ForNode::ForNode(Node* assign, Node* test, Node* iteration, BlockNode* body):
assign(assign), test(test), iteration(iteration), body(body) {
  if (test->_type() != BOOL) {
    AST::Node* _bool = new AST::BoolNode(0);
    errorMessage(if_test, _bool, test);
    delete _bool;
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

ForNode::~ForNode() {
  delete assign;
  delete test;
  delete iteration;
  delete body;
}
