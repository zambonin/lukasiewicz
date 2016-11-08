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

/* Bison standard error output function. */
extern void yyerror(const char* s, ...);

/* Saves the current indentation status. */
int spaces;

namespace AST {

/* Addition operator overload for NodeType enum. */
NodeType operator+(NodeType t, int v) {
  return static_cast<NodeType>(static_cast<int>(t) + v);
}

/* Subtraction operator overload for NodeType enum. */
NodeType operator-(NodeType t, int v) {
  return static_cast<NodeType>(static_cast<int>(t) - v);
}

template<typename T>
void text(const T& text, int n) {
  std::string blank(n, ' ');
  std::cout << blank << text;
}

std::string verboseType(Node* node, bool _short) {
  int n = node->_type();
  if (n < 0) {
    return "undefined";
  }
  std::string t = _short ? _var[n % 3] : _usr[n % 3];
  std::string ptr = _short ? " ref" : " pointer";

  while (n >= 6) {
    t += ptr;
    n -= 6;
  }

  t += (n >= 3) ? " array" : "";
  return t;
}

Node::Node() {
  this->type = ND;
}

Node::Node(int type) {
  this->type = static_cast<NodeType>(type);
}

void IntNode::print(bool /*prefix*/) {
  text(value, 1);
}

void FloatNode::print(bool /*prefix*/) {
  text(value, 1);
}

FloatNode::~FloatNode() {
  free(value);
}

void BoolNode::print(bool /*prefix*/) {
  text(value ? "true" : "false", 1);
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  // coercion enforcing
  if (left->_type() == INT && right->_type() == FLOAT && binOp != assign) {
    this->left = new UnaryOpNode(cast_float, left);
  } else if (left->_type() == FLOAT && right->_type() == INT) {
    this->right = new UnaryOpNode(cast_float, right);
  }

  // error handling
  bool differentTypes = (this->left->_type() != this->right->_type());
  bool bothValidNodes = ((this->left->_type() + this->right->_type()) >= 0);

  if (binOp == index) {
    if (this->left->_type() % 6 < 3) {
      yyerror("semantic error: variable %s is not an array",
        dynamic_cast<VariableNode*>(this->left)->id);
    } else if (this->right->_type() != INT) {
      yyerror(
        "semantic error: index operation expected integer but received %s",
        verboseType(this->right, false).c_str());
    }
  } else if (differentTypes && bothValidNodes) {
    yyerror("semantic error: %s operation expected %s but received %s",
      _opt[binOp].c_str(), verboseType(this->left, false).c_str(),
      verboseType(this->right, false).c_str());
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
  }

  // error handling
  if (op == ref && this->type < 0) {
    yyerror("semantic error: reference operation expects a pointer");
  } else if (op == addr && dynamic_cast<VariableNode*>(node) == nullptr) {
    AST::UnaryOpNode* test = dynamic_cast<UnaryOpNode*>(node);
    if (test != nullptr && test->op != index) {
      yyerror(
        "semantic error: address operation expects a variable or array item");
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

LinkedNode::~LinkedNode() {
  delete next;
}

void VariableNode::print(bool /*prefix*/) {
  if (next != nullptr) {
    next->print(false);
    text(",", 0);
  }
  std::string s;
  // prints the size of the array if the `size` attribute is not zero
  s = (this->size != 0) ? " (size: " + std::to_string(this->size) + ")" : "";
  text(id + s, 1);
}

VariableNode::~VariableNode() {
  free(id);
}

void BlockNode::print(bool prefix) {
  for (Node* n : nodeList) {
    n->print(prefix);
    if (dynamic_cast<FuncNode*>(n) == nullptr && n->_type() != ND) {
      text("\n", 0);
    }
  }
}

BlockNode::~BlockNode() {
  for (Node* n : nodeList) {
    delete n;
  }
}

void MessageNode::print(bool /*prefix*/) {
  std::string s = (this->_type() % 6 < 3) ? " var:" : ":";
  text(verboseType(this, true) + s, spaces);
  next->print(false);
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
condition(condition), _then(_then), _else(_else) {
  // error handling
  if (condition->_type() != BOOL) {
    yyerror(
      "semantic error: test operation expected boolean but received %s",
      verboseType(condition, false).c_str());
  }
}

void IfNode::print(bool /*prefix*/) {
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
  // error handling
  if (test->_type() != BOOL) {
    yyerror(
      "semantic error: test operation expected boolean but received %s",
      verboseType(test, false).c_str());
  }
}

void ForNode::print(bool /*prefix*/) {
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

  // error handling
  if (this->contents != nullptr) {
    AST::Node* ret = this->contents->nodeList.back();
    if (this->type != ret->_type()) {
      yyerror(
        "semantic error: function %s has incoherent return type", this->id);
    }
  }
}

void FuncNode::print(bool /*prefix*/) {
  if (this->contents != nullptr) {
    text(verboseType(this, true) + " fun: " + this->id + " (params: ", spaces);
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

std::deque<ParamNode*> ParamNode::createDeque() {
  std::deque<ParamNode*> v = {};
  ParamNode* l = this;
  while (l != nullptr) {
    v.push_front(l);
    l = dynamic_cast<ParamNode*>(l->next);
  }
  return v;
}

void ParamNode::print(bool /*prefix*/) {
  if (this->_type() != ND) {
    if (next != nullptr) {
      next->print(false);
      text(", ", 0);
    }
    text(verboseType(this, true) + " " + id, 0);
  }
}

void ReturnNode::print(bool /*prefix*/) {
  text("ret", spaces);
  next->print(true);
}

FuncCallNode::FuncCallNode(char* id, Node* function, BlockNode* params):
id(id), function(function), params(params) {
  this->type = function->type;

  // error handling
  std::vector<Node*> callParam = params->nodeList;
  int callSize = callParam.size();

  AST::FuncNode* f = dynamic_cast<AST::FuncNode*>(function);
  AST::ParamNode* l = dynamic_cast<ParamNode*>(f->params);
  std::deque<ParamNode*> origParam = {};
  if (l != nullptr) {
    origParam = l->createDeque();
  }
  int origSize = origParam.size();

  if (origSize != callSize) {
    yyerror(
      "semantic error: function %s expects %d parameters but received %d",
      id, origSize, callSize);
  } else {
    for (int i = 0; i < origSize; ++i) {
      if (origParam[i]->_type() != callParam[i]->_type()) {
        yyerror("semantic error: parameter %s expected %s but received %s",
          origParam[i]->id,
          verboseType(origParam[i], false).c_str(),
          verboseType(callParam[i], false).c_str());
      }
    }
  }
}

void FuncCallNode::print(bool /*prefix*/) {
  std::string fname(id);
  std::string psize = std::to_string(params->nodeList.size());
  text(" " + fname + "[" + psize + " params]", spaces);
  for (Node* n : params->nodeList) {
    n->print(true);
  }
}

FuncCallNode::~FuncCallNode() {
  free(id);
  delete params;
}

} // namespace AST
