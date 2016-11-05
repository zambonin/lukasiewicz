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

/* Addition operator overload for NodeType enum. */
NodeType operator+(NodeType t, int v) {
  return static_cast<NodeType>(static_cast<int>(t) + v);
}

/* Subtraction operator overload for NodeType enum. */
NodeType operator-(NodeType t, int v) {
  return static_cast<NodeType>(static_cast<int>(t) - v);
}

/* Prints an object with `cout`, prepended by `n` spaces. */
template<typename T>
void text(const T& text, int n) {
  std::string blank(n, ' ');
  std::cout << blank << text;
}

std::string Node::verboseType(bool _short) {
  int n = this->_type();
  std::string t = _short ? _var[n % 3] : _usr[n % 3];
  std::string ptr = _short ? " ref" : " pointer";

  while (n >= 6) {
    t += ptr;
    n -= 6;
  }

  t += (n >= 3) ? " array" : "";
  return t;
}

void Node::errorMessage(Operation op, Node* n1, Node* n2) {
  yyerror("semantic error: %s operation expected %s but received %s",
    _opt[op].c_str(), n1->verboseType(false).c_str(),
    n2->verboseType(false).c_str());
}

IntNode::IntNode(int value):
value(value) {
  this->type = INT;
}

void IntNode::print(bool prefix) {
  text(value, 1);
}

FloatNode::FloatNode(char* value):
value(value) {
  this->type = FLOAT;
}

void FloatNode::print(bool prefix) {
  text(value, 1);
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
    return left->_type() - 3;
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
    this->type = node->_type() - 6;
  } else if (op == addr) {
    this->type = node->_type() + 6;
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

UnaryOpNode::~UnaryOpNode() {
  delete node;
}

VariableNode::VariableNode(char* id, Node* next, int type, int size):
id(id), next(next), size(size) {
  this->type = static_cast<NodeType>(type);
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
    if (!dynamic_cast<FuncNode*>(n) && n->_type() != ND) {
      text("\n", 0);
    }
  }
}

BlockNode::~BlockNode() {
  for (Node* n : nodeList) {
    delete n;
  }
}

void MessageNode::print(bool prefix) {
  std::string s = (this->_type() % 6 < 3) ? " var:" : ":";
  text(this->verboseType(true) + s, spaces);
  node->print(false);
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

FuncNode::FuncNode(char* id, Node* params, int type, BlockNode* contents):
id(id), params(params), contents(contents) {
  this->type = static_cast<NodeType>(type);
  if (this->contents != nullptr) {
    AST::Node* ret = this->contents->nodeList.back();
    if (this->type != ret->_type()) {
      yyerror(
        "semantic error: function %s has incoherent return type", this->id);
    }
  }
}

void FuncNode::print(bool prefix) {
  if (this->contents != nullptr) {
    text(this->verboseType(true) + " fun: " + this->id + " (params: ", spaces);
    params->print(false);
    text(")\n", 0);
    _tab(contents->print(true));
  } else {
    yyerror(
      "semantic error: function %s is declared but never defined", this->id);
  }
}

FuncNode::~FuncNode() {
  free(id);
  delete params;
  delete contents;
}

void ParamNode::print(bool prefix) {
  if (next != nullptr) {
    next->print(false);
    text(", ", 0);
  }
  text(this->verboseType(true) + " " + id, 0);
}

void ReturnNode::print(bool prefix) {
  text("ret", spaces);
  node->print(true);
}
