#include "ast.h"

namespace AST {

/* Verbose representation for the node types. */
static const std::string _usr[] = {
  "integer", "float", "boolean", "character"
};

/* Basic representation for the node types. */
static const std::string _var[] = {
  "int", "float", "bool", "char"
};

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

Node::Node(int type) {
  this->type = static_cast<NodeType>(type);
}

std::string Node::_vtype(bool _short) {
  int n = this->_type();
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

NodeType CharNode::_type() {
  return (value[0] == '\"') ? this->type + 4 : this->type;
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
Node(), binOp(binOp), left(left), right(right) {
  // transpiler needs to know if the variable was initialized
  VariableNode* v = dynamic_cast<VariableNode*>(left);
  if (binOp == assign && v != nullptr) {
    v->init = true;
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
  this->error_handler();
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
Node(), op(op), node(node) {
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
  this->error_handler();
}

UnaryOpNode::~UnaryOpNode() {
  delete node;
}

LinkedNode::~LinkedNode() {
  delete next;
}

BlockNode::BlockNode(Node* n) : Node() {
  if (n != nullptr) {
    nodeList.push_back(n);
  }
}

BlockNode::~BlockNode() {
  for (Node* n : nodeList) {
    delete n;
  }
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
Node(), condition(condition), _then(_then), _else(_else) {
  this->error_handler();
}

IfNode::~IfNode() {
  delete condition;
  delete _then;
  delete _else;
}

ForNode::ForNode(Node* assign, Node* test, Node* iteration, BlockNode* body):
Node(), assign(assign), test(test), iteration(iteration), body(body) {
  this->error_handler();
}

ForNode::~ForNode() {
  delete assign;
  delete test;
  delete iteration;
  delete body;
}

FuncNode::FuncNode(std::string id, Node* params, int type, BlockNode* contents):
Node(type), id(std::move(id)), params(params), contents(contents) {
  this->error_handler();
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

FuncCallNode::FuncCallNode(FuncNode* function, BlockNode* params):
Node(), function(function), params(params) {
  this->error_handler();
}


NodeType FuncCallNode::_type() {
  return this->function->_type();
}

FuncCallNode::~FuncCallNode() {
  delete params;
}

HiOrdFuncNode::HiOrdFuncNode(std::string id, Node* func, VariableNode* array):
FuncNode(array->id + "_" + id, new ParamNode(array->id, nullptr,
  array->_type(), array->size), array->_type(), new BlockNode(func)) {
  this->hi_error_handler(array);
}

HiOrdFuncNode* HiOrdFuncNode::chooseFunc(
  const std::string& id, Node* func, VariableNode* array) {
  if (id == "map") {
    return new MapFuncNode(id, func, array);
  }
  if (id == "fold") {
    return new FoldFuncNode(id, func, array);
  }
  if (id == "filter") {
    return new FilterFuncNode(id, func, array);
  }
  return nullptr;
}

MapFuncNode::MapFuncNode(std::string fid, Node* func, VariableNode* array):
HiOrdFuncNode(fid, func, array) {
  std::string id = array->id, ti = id + "_ti", ta = id + "_ta";
  std::ostringstream out;
  int n = array->_type(), s = array->size;

  auto tmp = new Node(n - 4);
  std::string t = (n < 3) ? "int" : tmp->_vtype(true);
  delete tmp;

  out << "int " << ti << "\n" << t << " " << ta << "[" << s << "]\nfor "
    << ti << " = 0, " << ti << " < [len] " << id << ", " << ti << " = "
    << ti << " + 1 {\n  " << ta << "[" << ti << "] = λ(" << id << "["
    << ti << "])\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(ta, nullptr, n, s);
  this->contents->nodeList.push_back(new ReturnNode(v));
  this->hi_error_handler(func);
}

FoldFuncNode::FoldFuncNode(std::string fid, Node* func, VariableNode* array):
HiOrdFuncNode(fid, func, array) {
  this->type = this->type - 4;
  std::string id = array->id, ti = id + "_ti", tv = id + "_tv";
  std::ostringstream out;

  auto tmp = new Node(array->_type() - 4);
  out << tmp->_vtype(true) << " " << tv << "\n" << tv << " = " << id
    << "[0]\nint " << ti << "\nfor " << ti << " = 1, " << ti << " < [len] "
    << id << ", " << ti << " = " << ti << " + 1 {\n  " << tv << " = " << tv
    << " + λ(" << tv << ", " << array->id << "[" << ti << "])\n}\n";
  delete tmp;

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(tv, nullptr, array->_type() % 4, 0);
  this->contents->nodeList.push_back(new ReturnNode(v));
  this->hi_error_handler(func);
}

FilterFuncNode::FilterFuncNode(std::string fid, Node* func,
  VariableNode* array): HiOrdFuncNode(fid, func, array) {
  std::string id = array->id, ti = id + "_ti", ta = id + "_ta";
  std::ostringstream out;
  int n = array->_type();

  auto tmp = new Node(n - 4);
  std::string t = (n < 3) ? "int" : tmp->_vtype(true);
  delete tmp;

  out << "int " << ti << "\n" << t << " " << ta << "[0]\nfor " << ti
    << " = 0, " << ti << " < [len] " << id << ", " << ti << " = " << ti
    << " + 1 {\n  if λ(" << id << "[" << ti << "])\n  then {\n    "
    << ta << " <- " << id << "[" << ti << "]\n  }\n}\n";

  this->contents->nodeList.push_back(string_read(out.str().c_str()));
  VariableNode* v = new VariableNode(ta, nullptr, n, array->size);
  this->contents->nodeList.push_back(new ReturnNode(v));
  this->hi_error_handler(func);
}

} // namespace AST
