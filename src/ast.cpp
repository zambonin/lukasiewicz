#include "ast.h"

extern void yyerror(const char* s, ...);
extern AST::BlockNode* string_read(const char* s);

/* Macros that reduce the visual pollution when indentation is needed. */

/* Takes a single line of code and indents it with two spaces. */
#define _tab(X)     spaces += 2; (X); spaces -= 2

/* Variadic macro that prevents indentation for any number of lines. */
#define _notab(...) int tmp = spaces; spaces = 0; (__VA_ARGS__); spaces = tmp;

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

/*
 * `Equal to` operator overload for ParamNode. Two ParamNodes
 * are the same if they have the same id and type.
 */
bool operator==(const ParamNode& n1, const ParamNode& n2) {
  std::string s1(n1.id), s2(n2.id);
  return s1 == s2 && n1.type == n2.type;
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
  std::string t = _short ? _var[n % 4] : _usr[n % 4];
  std::string ptr = _short ? " ref" : " pointer";

  while (n >= 8) {
    t += ptr;
    n -= 8;
  }

  t += (n >= 4) ? " array" : "";
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

void CharNode::print(bool /*prefix*/) {
  text(value, 1);
}

NodeType CharNode::_type() {
  return (value[0] == '\"') ? this->type + 4 : this->type;
}

CharNode::~CharNode() {
  free(value);
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  // coercion enforcing
  if (binOp != assign) {
    if (left->_type() == INT && right->_type() == FLOAT) {
      this->left = new UnaryOpNode(cast_float, left);
    } else if (left->_type() == CHAR && right->_type() == A_CHAR) {
      this->left = new UnaryOpNode(cast_word, left);
    } else if (left->_type() == CHAR && right->_type() == CHAR) {
      this->left = new UnaryOpNode(cast_word, left);
      this->right = new UnaryOpNode(cast_word, right);
    }
  }
  if (left->_type() == FLOAT && right->_type() == INT) {
    this->right = new UnaryOpNode(cast_float, right);
  } else if (left->_type() == A_CHAR && right->_type() == CHAR) {
    this->right = new UnaryOpNode(cast_word, right);
  }

  // error handling
  bool differentTypes = (this->left->_type() != this->right->_type());
  bool bothValid = (this->left->_type() >= 0 && this->right->_type() >= 0);

  if (binOp == index) {
    if ((this->left->_type() % 8) < 4) {
      yyerror("semantic error: variable %s is not an array",
        dynamic_cast<VariableNode*>(this->left)->id.c_str());
    } else if (this->right->_type() != INT) {
      yyerror(
        "semantic error: index operation expected integer but received %s",
        verboseType(this->right, false).c_str());
    }
  } else if (differentTypes && bothValid) {
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
    return left->_type() - 4;
  }
  return (binOp < 8) ? left->_type() : BOOL;
}

BinaryOpNode::~BinaryOpNode() {
  delete left;
  delete right;
}

UnaryOpNode::UnaryOpNode(Operation op, Node* node):
op(op), node(node) {
  if (op == cast_int || op == len) {
    this->type = INT;
  } else if (op == cast_float) {
    this->type = FLOAT;
  } else if (op == cast_bool || op == _not) {
    this->type = BOOL;
  } else if (op == cast_word) {
    this->type = A_CHAR;
  } else if (op == uminus) {
    this->type = node->_type();
  } else if (op == ref) {
    this->type = node->_type() - 8;
  } else if (op == addr) {
    this->type = node->_type() + 8;
  }

  // error handling
  if (op == ref && this->type < 0) {
    yyerror("semantic error: reference operation expects a pointer");
  } else if (op == len && (node->_type() % 6) < 3) {
    yyerror("semantic error: length operation expects an array");
  } else if (op == addr) {
    bool isNotVar = (dynamic_cast<VariableNode*>(node) == nullptr);
    AST::BinaryOpNode* indexNode = dynamic_cast<BinaryOpNode*>(node);
    bool isNotIndex = (indexNode != nullptr && indexNode->binOp != index);
    if ((indexNode == nullptr && isNotVar) || isNotIndex) {
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
  text(id, 1);
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
  std::string s = ((this->_type() % 8) < 4) ? " var:" : ":";
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

FuncNode::FuncNode(std::string id, Node* params, int type, BlockNode* contents):
Node(type), id(id), params(params), contents(contents) {
  // error handling
  if (this->contents != nullptr) {
    Node* ret = this->contents->nodeList.back();
    if (this->type != ret->_type()) {
      yyerror("semantic error: function %s has incoherent return type",
        this->id.c_str());
    }
  }
}

void FuncNode::print(bool /*prefix*/) {
  if (this->contents != nullptr) {
    text(verboseType(this, true) + " fun: " + this->id + " (params: ", spaces);
    if (params != nullptr) {
      params->print(false);
    }
    text(")\n", 0);
    _tab(contents->print(true));
  } else {
    yyerror("semantic error: function %s is declared but never defined",
      this->id.c_str());
  }
}

bool FuncNode::verifyParams(Node* n) {
  ParamNode* a = dynamic_cast<ParamNode*>(this->params);
  ParamNode* b = dynamic_cast<ParamNode*>(n);

  bool sameNode = true;
  while (sameNode && a != nullptr && b != nullptr) {
    sameNode = (*a == *b);
    a = dynamic_cast<ParamNode*>(a->next);
    b = dynamic_cast<ParamNode*>(b->next);
  }

  return a == nullptr && b == nullptr && sameNode;
}

FuncNode::~FuncNode() {
  delete params;
  delete contents;
}

std::deque<VariableNode*> VariableNode::createDeque() {
  std::deque<VariableNode*> v = {};
  VariableNode* l = this;
  while (l != nullptr) {
    v.push_front(l);
    l = dynamic_cast<VariableNode*>(l->next);
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
  _notab(next->print(true));
}

FuncCallNode::FuncCallNode(FuncNode* function, BlockNode* params):
function(function), params(params) {
  this->type = function->type;

  // error handling
  std::vector<Node*> callParam = params->nodeList;
  int callSize = callParam.size();

  VariableNode* l = dynamic_cast<VariableNode*>(function->params);
  std::deque<VariableNode*> origParam = {};
  if (l != nullptr) {
    origParam = l->createDeque();
  }
  int origSize = origParam.size();

  if (origSize != callSize) {
    yyerror(
      "semantic error: function %s expects %d parameters but received %d",
      function->id.c_str(), origSize, callSize);
  } else {
    for (int i = 0; i < origSize; ++i) {
      if (origParam[i]->_type() != callParam[i]->_type()) {
        yyerror("semantic error: parameter %s expected %s but received %s",
          origParam[i]->id.c_str(),
          verboseType(origParam[i], false).c_str(),
          verboseType(callParam[i], false).c_str());
      }
    }
  }
}

void FuncCallNode::print(bool /*prefix*/) {
  std::string psize = std::to_string(params->nodeList.size());
  text(" " + function->id + "[" + psize + " params]", spaces);
  for (Node* n : params->nodeList) {
    n->print(true);
  }
}

FuncCallNode::~FuncCallNode() {
  delete params;
}

void DeclarationNode::print(bool prefix) {
  if (next != nullptr) {
    next->print(false);
    text(",", 0);
  }
  std::string s;
  // prints the size of the array if the `size` attribute is not zero
  s = (this->size != 0) ? " (size: " + std::to_string(this->size) + ")" : "";
  text(id + s, 1);
}

MapFuncNode::MapFuncNode(VariableNode* array, Node* func):
FuncNode("map", nullptr, array->_type(), nullptr), func(func) {
  this->id = array->id + "_" + this->id;
  this->params = new ParamNode(array->id, nullptr, array->_type(), array->size);

  this->contents = new BlockNode();
  this->contents->nodeList.push_back(func);
  expandBody(array);
  VariableNode* v = new VariableNode(
    array->id + "_ta", nullptr, array->_type(), array->size);
  ReturnNode* _ret = new ReturnNode(v, v->_type());
  this->contents->nodeList.push_back(_ret);

  // error handling
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if ((array->_type() % 8) < 4) {
    yyerror("semantic error: second parameter must be an array");
  }
  if (f->_type() != (this->_type() % 4)) {
    yyerror("semantic error: function %s has incoherent return type",
      f->id.c_str());
  }
}

void MapFuncNode::expandBody(VariableNode* array) {
  std::string ti = array->id + "_ti", ta = array->id + "_ta";

  std::ostringstream out;
  out << "int " << ti << "\n"                                                 \
    << _var[array->_type() - 4] << " " << ta << "[" << array->size << "]\n"   \
    << "for " << ti << " = 0, " << ti << " < [len] t, "                       \
    << ti << " = " << ti << " + 1 {\n  "                                      \
    << ta << "[" << ti << "] = λ(" << array->id << "[" << ti << "])\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
}

} // namespace AST
