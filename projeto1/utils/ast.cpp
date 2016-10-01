#include "ast.h"
#include "st.h"

using namespace AST;

extern void yyerror(const char* s, ...);
extern ST::SymbolTable* current;


int spaces;

template<typename T>
void text(const T& text, int n) {
  std::string blank(n, ' ');
  std::cout << blank << text;
}

void Node::errorMessage(Operation op, Node* n1, Node* n2) {
  yyerror("semantic error: %s operation expected %s but received %s\n",
          errorMsg[op].c_str(), nodeName[n1->_type()].c_str(),
          nodeName[n2->_type()].c_str());
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
  if (left->_type() != right->_type()) {
    if (left->_type() == INT && right->_type() == FLOAT && binOp != assign) {
      this->left = new UnaryOpNode(cast_float, left);
    } else if (left->_type() == FLOAT && right->_type() == INT) {
      this->right = new UnaryOpNode(cast_float, right);
    } else {
      if(left->_type() != ND) {
        errorMessage(binOp, left, right);
      }
    }
  }
}

void BinaryOpNode::print(bool prefix) {
  if (prefix) {
    text("", binOp != assign);
    text(strOp[binOp], spaces);
    int tmp = spaces;
    spaces = 0;
    left->print(prefix);
    right->print(prefix);
    spaces = tmp;
  } else {
    int tmp = spaces;
    spaces = 0;
    left->print(!prefix);
    text("", binOp == assign);
    text(strOp[binOp], spaces);
    text("", binOp != assign);
    right->print(!prefix);
    spaces = tmp;
  }
}

NodeType BinaryOpNode::_type() {
  if (binOp == add || binOp == sub || binOp == mul
      || binOp == div || binOp == assign) {
    return left->_type();
  } else {
    return BOOL;
  }
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
  text(strOp[op], 0);
  node->print(prefix);
}

NodeType UnaryOpNode::_type() {
  return this->type;
}

void VariableNode::print(bool prefix) {
  if (next != NULL) {
    next->print(false);
    text(",", 0);
  }
  text(id, 1);
}

NodeType VariableNode::_type() {
  return type;
}

void BlockNode::print(bool prefix) {
  for (Node* n : nodeList) {
    n->print(prefix);
    if (n->_type() != BASIC) {
      text("\n", 0);
    }
  }
}

void MessageNode::print(bool prefix) {
  std::string t;
  if (this->_type() == INT) {
    t = "int";
  } else if (this->_type() == FLOAT) {
    t = "float";
  } else if (this->_type() == BOOL) {
    t = "bool";
  }
  text(t + " var:", spaces);
  node->print(false);
}

NodeType MessageNode::_type() {
  return node->_type();
}

IfNode::IfNode(Node* condition, BlockNode* _then, BlockNode* _else):
condition(condition), _then(_then), _else(_else) {
  if (condition->_type() != BOOL) {
    errorMessage(_if, new AST::BoolNode(NULL), condition);
  }
}

void IfNode::print(bool prefix) {
  text("if:", spaces);
  int tmp_tab = spaces;
  spaces = 0;
  condition->print(true);
  spaces = tmp_tab;
  text("\n", 0);
  text("then:\n", spaces);
  spaces += 2;
  _then->print(true);
  spaces -= 2;
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    spaces += 2;
    _else->print(true);
    spaces -= 2;
  }
}

void ForNode::print(bool prefix) {
  text("for: ", spaces);
  int tmp_tab = spaces;
  spaces = 0;
  this->assignNode->print(true);

  text(",", 0);
  this->testNode->print(true);

  text(",", 0);
  if (itNode->_type() != BASIC) {
    text("", 1);
  }

  this->itNode->print(true);
  spaces = tmp_tab;

  text("\n", 0);
  text("do:\n", spaces);
  spaces += 2;
  this->doNode->print(true);
  spaces -= 2;

}
