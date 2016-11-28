#include "ast.h"

extern void yyerror(const char* s, ...);
extern void yyserror(const char* s, ...);
extern AST::BlockNode* string_read(const char* s);

/* Macros that reduce the visual pollution when indentation is needed. */

/* Takes a single line of code and indents it with two spaces. */
#define _tab(X)     spaces += 2; (X); spaces -= 2

/* Variadic macro that prevents indentation for any number of lines. */
#define _notab(...) int tmp = spaces; spaces = 0; (__VA_ARGS__); spaces = tmp;

/* Checks if a node is an array. */
#define notArray(X) (((X)->_type() % 8) < 4)

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

/* Modulo operator overload for NodeType enum. */
NodeType operator%(NodeType t, int v) {
  return static_cast<NodeType>(static_cast<int>(t) % v);
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

void IntNode::printInfix() {
  text(value, 1);
}

void IntNode::printPython() {
  text(value, 0);
}

void FloatNode::printInfix() {
  text(value, 1);
}

void FloatNode::printPython() {
  text(value, 0);
}

void BoolNode::printInfix() {
  text(value ? "true" : "false", 1);
}

void BoolNode::printPython() {
  text(value ? "True" : "False", 0);
}

void CharNode::printInfix() {
  text(value, 1);
}

void CharNode::printPython() {
  text(value, 0);
}

NodeType CharNode::_type() {
  return (value[0] == '\"') ? this->type + 4 : this->type;
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  if (binOp == assign) {
    VariableNode* v = dynamic_cast<VariableNode*>(left);
    if (v) v->init = true;
  }

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
  VariableNode* v1 = dynamic_cast<VariableNode*>(this->left);
  VariableNode* v2 = dynamic_cast<VariableNode*>(this->right);
  FuncCallNode* f1 = dynamic_cast<FuncCallNode*>(this->right);
  int n = 0;

  if (v2 != nullptr) {
    n = v2->size;
  } else if (f1 != nullptr && !notArray(f1->function)) {
    n = dynamic_cast<VariableNode*>(dynamic_cast<ReturnNode*>(
      f1->function->contents->nodeList.back())->next)->size;
  }
  if (v1 != nullptr && (v1->size < n)) {
    yyserror("operation between mismatched array sizes");
  }

  if (left->_type() == A_CHAR && right->_type() == A_CHAR) {
    CharNode* c = dynamic_cast<CharNode*>(right);
    if (c != nullptr && v1 != nullptr && v1->size < c->value.size() - 2) {
      c->value.resize(v1->size + 1);
      c->value += "\"";
      yyerror("warning: value truncated to %s", c->value.c_str());
    }
  }

  bool differentTypes = (this->left->_type() != this->right->_type());
  bool bothValid = (this->left->_type() >= 0 && this->right->_type() >= 0);

  if (binOp == index) {
    if (notArray(this->left)) {
      yyserror("left hand side of index operation is not an array");
    } else if (this->right->_type() != INT) {
      yyserror("index operation expected integer but received %s",
        verboseType(this->right, false).c_str());
    }
  } else if (binOp == append) {
    if (notArray(this->left)) {
      yyserror("left hand side of append operation is not an array");
    } else if ((this->left->_type() % 4) != this->right->_type()) {
      yyserror("append operation expected %s but received %s",
        verboseType(new Node(this->left->_type() % 4), false).c_str(),
        verboseType(this->right, false).c_str());
    } else {
      dynamic_cast<VariableNode*>(this->left)->size++;
    }
  } else if (differentTypes && bothValid) {
    yyserror("%s operation expected %s but received %s",
      _opt[binOp].c_str(), verboseType(this->left, false).c_str(),
      verboseType(this->right, false).c_str());
  }
}

void BinaryOpNode::printPrefix() {
  bool space = ((binOp != assign) && (binOp != append));
  text("", space);
  text(_bin[binOp], spaces);
  _notab(
    left->printPrefix(),
    right->printPrefix());
}

void BinaryOpNode::printInfix() {
  bool space = ((binOp != assign) && (binOp != append));
  _notab(
    left->printInfix(),
    text("", !space),
    text(_bin[binOp], spaces),
    right->printInfix());
}

void BinaryOpNode::printPython() {
  if (binOp != assign && binOp != index) text("(", 0);
  left->printPython();
  if (binOp != index) {
    text("", 1);
    text(_bin[binOp], 0),
    text("", 1);
    right->printPython();
  } else {
    text("[", 0);
    right->printPython();
    text("]", 0);
  }
  if (binOp != assign && binOp != index) text(")", 0);
}


NodeType BinaryOpNode::_type() {
  if (binOp == index || binOp == append) {
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
    yyserror("reference operation expects a pointer");
  } else if (op == len && notArray(node)) {
    yyserror("length operation expects an array");
  } else if (op == addr) {
    bool isNotVar = (dynamic_cast<VariableNode*>(node) == nullptr);
    AST::BinaryOpNode* indexNode = dynamic_cast<BinaryOpNode*>(node);
    bool isNotIndex = (indexNode != nullptr && indexNode->binOp != index);
    if ((indexNode == nullptr && isNotVar) || isNotIndex) {
       yyserror("address operation expects a variable or array item");
     }
  }
}

void UnaryOpNode::printInfix() {
  this->printPrefix();
}

void UnaryOpNode::printPrefix() {
  text(_bin[op], 1);
  node->printPrefix();
}

void UnaryOpNode::printPython() {
  text(_binp[op], 0);
  node->printPython();
  if (op > 16) {
    text(")", 0);
  }
}

UnaryOpNode::~UnaryOpNode() {
  delete node;
}

LinkedNode::~LinkedNode() {
  delete next;
}

void VariableNode::printInfix() {
  text(id, 1);
}

void VariableNode::printPython() {
  text(id, 0);
}

BlockNode::BlockNode(Node* n) {
  if (n != nullptr) {
    nodeList.push_back(n);
  }
}

void BlockNode::printPrefix() {
  for (Node* n : nodeList) {
    if (n != nullptr) {
      n->printPrefix();
      if (dynamic_cast<FuncNode*>(n) == nullptr && n->_type() != ND) {
        text("\n", 0);
      }
    }
  }
}

void BlockNode::printPython() {
  for (Node* n : nodeList) {
    if (n != nullptr) {
      text("", spaces);
      n->printPython();
      if (n->_type() != ND) {
        text("\n", 0);
      }
    }
  }
}

BlockNode::~BlockNode() {
  for (Node* n : nodeList) {
    delete n;
  }
}

void MessageNode::printPrefix() {
  std::string s = (notArray(this)) ? " var:" : ":";
  text(verboseType(this, true) + s, spaces);
  next->printInfix();
}

void MessageNode::printPython() {
  next->printPython();
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
condition(condition), _then(_then), _else(_else) {
  // error handling
  if (condition->_type() != BOOL) {
    yyserror("test operation expected boolean but received %s",
      verboseType(condition, false).c_str());
  }
}

void IfNode::printPrefix() {
  text("if:", spaces);
  _notab(condition->printPrefix());
  text("\n", 0);
  text("then:\n", spaces);
  _tab(_then->printPrefix());
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    _tab(_else->printPrefix());
  }
}

void IfNode::printPython() {
  text("if ", 0);
  _notab(condition->printPython());
  text(":\n", 0);
  _tab(_then->printPython());
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    _tab(_else->printPython());
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
    yyserror("test operation expected boolean but received %s",
      verboseType(test, false).c_str());
  }
}

void ForNode::printPrefix() {
  text("for: ", spaces);
  _notab(
    assign->printPrefix(),
    text(",", 0),
    test->printPrefix(),
    text(", ", 0),
    iteration->printPrefix());
  text("\n", 0);
  text("do:\n", spaces);
  _tab(body->printPrefix());
}

void ForNode::printPython() {
  assign->printPython();
  if (assign->_type() != ND) text("\n", 0);
  text("while ", spaces);
  _notab(
    test->printPython(),
    text(":\n", 0));
  _tab(body->printPython());
  if (iteration->_type() != ND) {
    text("", spaces + 2);
    iteration->printPython();
    text("\n", 0);
  }
}

ForNode::~ForNode() {
  delete assign;
  delete test;
  delete iteration;
  delete body;
}

FuncNode::FuncNode(std::string id, Node* params, int type, BlockNode* contents):
Node(type), id(std::move(id)), params(params), contents(contents) {
  // error handling
  if (this->contents != nullptr) {
    Node* ret = this->contents->nodeList.back();
    bool isReturn = (dynamic_cast<ReturnNode*>(ret) != nullptr);
    if (this->type != ret->_type() && isReturn) {
      yyserror("function %s has incoherent return type", this->id.c_str());
    }
  }
}

void FuncNode::printPrefix() {
  if (this->contents != nullptr) {
    text(verboseType(this, true) + " fun: " + this->id + " (params: ", spaces);
    if (params != nullptr) {
      params->printInfix();
    }
    text(")\n", 0);
    _tab(contents->printPrefix());
  } else {
    yyserror("function %s is declared but never defined", this->id.c_str());
  }
}

void FuncNode::printPython() {
  text("def " + this->id + "(", 0);
  if (params != nullptr) {
    params->printPython();
  }
  text("):\n", 0);
  if (this->contents != nullptr) {
    _tab(contents->printPython());
  } else {
    text("pass", spaces);
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

std::deque<VariableNode*> FuncNode::createDeque() {
  std::deque<VariableNode*> v = {};
  VariableNode* l = dynamic_cast<VariableNode*>(this->params);
  while (l != nullptr) {
    v.push_front(l);
    l = dynamic_cast<VariableNode*>(l->next);
  }
  return v;
}

void ParamNode::printInfix() {
  if (this->_type() != ND) {
    if (next != nullptr) {
      next->printInfix();
      text(", ", 0);
    }
    text(verboseType(this, true) + " " + id, 0);
  }
}

void ParamNode::printPython() {
  if (this->_type() != ND) {
    if (next != nullptr) {
      next->printPython();
      text(", ", 0);
    }
    text(id, 0);
  }
}

void ReturnNode::printPrefix() {
  text("ret", spaces);
  _notab(next->printPrefix());
}

void ReturnNode::printPython() {
  text("return ", 0);
  _notab(next->printPython());
}

FuncCallNode::FuncCallNode(FuncNode* function, BlockNode* params):
function(function), params(params) {
  // error handling
  std::vector<Node*> callParam = params->nodeList;
  int callSize = callParam.size();

  std::deque<VariableNode*> origParam = function->createDeque();
  int origSize = origParam.size();

  if (origSize != callSize) {
    yyserror("function %s expects %d parameters but received %d",
      function->id.c_str(), origSize, callSize);
  } else {
    for (int i = 0; i < origSize; ++i) {
      if (origParam[i]->_type() != callParam[i]->_type()) {
        yyserror("parameter %s expected %s but received %s",
          origParam[i]->id.c_str(),
          verboseType(origParam[i], false).c_str(),
          verboseType(callParam[i], false).c_str());
      }
    }
  }
}

void FuncCallNode::printPrefix() {
  std::string psize = std::to_string(params->nodeList.size());
  text(" " + function->id + "[" + psize + " params]", spaces);
  for (Node* n : params->nodeList) {
    n->printPrefix();
  }
}

void FuncCallNode::printPython() {
  text(function->id + "(", 0);
  for (Node* n: params->nodeList) {
    n->printPython();
    if (n != params->nodeList.back()) text(", ", 0);
  }
  text(")", 0);
}

NodeType FuncCallNode::_type() {
  return this->function->_type();
}

FuncCallNode::~FuncCallNode() {
  delete params;
}

void DeclarationNode::printInfix() {
  if (next != nullptr) {
    next->printInfix();
    text(",", 0);
  }
  std::string s = "";
  if (!notArray(this)) {
    s = " (size: " + std::to_string(this->size) + ")";
  }
  text(id + s, 1);
}

void DeclarationNode::printPython() {
  if (next != nullptr) {
    next->printPython();
    text("\n", 0);
  }
  std::string s = (this->init) ? "" : " = None";
  s = (!notArray(this)) ? " = [None] * " + std::to_string(this->size) : s;
  text(id + s, 0);
}

HiOrdFuncNode::HiOrdFuncNode(std::string id, Node* func, VariableNode* array):
FuncNode(array->id + "_" + id, new ParamNode(array->id, nullptr,
  array->_type(), array->size), array->_type(), new BlockNode(func)) {
  // error handling
  if (notArray(array)) {
    yyserror("second parameter must be of array type");
  }
}

HiOrdFuncNode* HiOrdFuncNode::chooseFunc(
  std::string id, Node* func, VariableNode* array) {
  if (id == "map") {
    return new MapFuncNode(id, func, array);
  } else if (id == "fold") {
    return new FoldFuncNode(id, func, array);
  } else if (id == "filter") {
    return new FilterFuncNode(id, func, array);
  }
  return nullptr;
}

MapFuncNode::MapFuncNode(std::string id, Node* func, VariableNode* array):
HiOrdFuncNode(id, func, array) {
  expandBody(array);
  // error handling
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != (this->type % 4)) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 1) {
    yyserror("map lambda expects 1 parameters but received %d", n);
  }
}

void MapFuncNode::expandBody(VariableNode* array) {
  std::string id = array->id, ti = id + "_ti", ta = id + "_ta";
  std::ostringstream out;
  int n = array->_type(), s = array->size;
  std::string t = (n < 3) ? "int" : _var[n - 4];

  out << "int " << ti << "\n" << t << " " << ta << "[" << s
    << "]\nfor " << ti << " = 0, " << ti << " < [len] " << id << ", "
    << ti << " = " << ti << " + 1 {\n  " << ta << "[" << ti << "] = λ("
    << id << "[" << ti << "])\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(ta, nullptr, n, s);
  this->contents->nodeList.push_back(new ReturnNode(v));
}

FoldFuncNode::FoldFuncNode(std::string id, Node* func, VariableNode* array):
HiOrdFuncNode(id, func, array) {
  this->type = this->type % 4;
  expandBody(array);
  // error handling
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != this->type) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 2) {
    yyserror("fold lambda expects 2 parameters but received %d", n);
  }
}

void FoldFuncNode::expandBody(VariableNode* array) {
  std::string id = array->id, ti = id + "_ti", tv = id + "_tv";
  std::ostringstream out;
  int n = array->_type() % 4;

  out << _var[n] << " " << tv << "\n" << tv << " = " << id << "[0]\nint "
    << ti << "\nfor " << ti << " = 1, " << ti << " < [len] " << id << ", "
    << ti << " = " << ti << " + 1 {\n  " << tv << " = " << tv << " + λ("
    << tv << ", " << array->id << "[" << ti << "])\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(tv, nullptr, n, 0);
  this->contents->nodeList.push_back(new ReturnNode(v));
}

FilterFuncNode::FilterFuncNode(std::string id, Node* func,
  VariableNode* array): HiOrdFuncNode(id, func, array) {
  expandBody(array);
  // error handling
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != BOOL) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 1) {
    yyserror("filter lambda expects 1 parameters but received %d", n);
  }
}

void FilterFuncNode::expandBody(VariableNode* array) {
  std::string id = array->id, ti = id + "_ti", ta = id + "_ta";
  std::ostringstream out;
  int n = array->_type();
  std::string t = (n < 3) ? "int" : _var[n - 4];

  out << "int " << ti << "\n" << t << " " << ta << "[0]\nfor " << ti
    << " = 0, " << ti << " < [len] " << id << ", " << ti << " = " << ti
    << " + 1 {\n  if λ(" << id << "[" << ti << "])\n  then {\n    "
    << ta << " <- " << id << "[" << ti << "]\n  }\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(ta, nullptr, n, array->size);
  this->contents->nodeList.push_back(new ReturnNode(v));
}

} // namespace AST
