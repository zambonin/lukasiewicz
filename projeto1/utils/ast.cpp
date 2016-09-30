#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

CondNode::CondNode(Node* boolExpr):
boolExpr(boolExpr) {
  if (boolExpr->_type() != BOOL) {
    errorMessage(_if, new AST::BoolNode(NULL), boolExpr);
  }
}

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  if (left->_type() != right->_type()) {
    if (left->_type() == INT && right->_type() == FLOAT && binOp != assign) {
      Node* leftCoercion = new UnaryOpNode(cast_float, left);
      this->left = leftCoercion;
    } else if (left->_type() == FLOAT && right->_type() == INT) {
      Node* rightCoercion = new UnaryOpNode(cast_float, right);
      this->right = rightCoercion;
    } else {
      errorMessage(binOp, left, right);
    }
  }
}

NodeType BinaryOpNode::_type() {
  if (binOp == add || binOp == sub || binOp == mul || binOp == div) {
    return left->_type();
  } else {
    return BOOL;
  }
}

UnaryOpNode::UnaryOpNode(Operation op, Node* node):
op(op), node(node) {
  if (op == cast_int) {
    this->_nodeType = INT;
  } else if (op == cast_float) {
    this->_nodeType = FLOAT;
  } else if (op == cast_bool || op == _not) {
    this->_nodeType = BOOL;
  } else if (op == uminus) {
    this->_nodeType = node->_type();
  }
}

AssignmentNode::AssignmentNode(Node* left, Node* right):
binOp(assign), left(left), right(right) {
  if (left->_type() != right->_type()) {
     if (left->_type() == FLOAT && right->_type() == INT) {
      Node* rightCoercion = new UnaryOpNode(cast_float, right);
      this->right = rightCoercion;
    } else {
      errorMessage(binOp, left, right);
    }
  }
}

NodeType VariableNode::_type() {
  std::string s = symbolTable.getSymbolType(this->id);
  return nodeTypeString[s];
}

void BinaryOpNode::printTree() {
  left->printTree();
  std::cout << strOp[binOp];
  if (binOp != assign) {
    std::cout << " ";
  }
  right->printTree();
}

void BinaryOpNode::printTreePrefix() {
  if (binOp != assign) {
    std::cout << " ";
  }
  std::cout << strOp[binOp];
  left->printTreePrefix();
  right->printTreePrefix();
}

void UnaryOpNode::printTree() {
  std::cout << strOp[op];
  node->printTree();
}

void UnaryOpNode::printTreePrefix() {
  std::cout << strOp[op];
  node->printTreePrefix();
}

void AssignmentNode::printTree() {
  left->printTree();
  std::cout << " =";
  right->printTree();
}

void AssignmentNode::printTreePrefix() {
  this->printTree();
}

void VariableNode::printTree() {
  if (next != NULL) {
    next->printTree();
    std::cout << ",";
  }
  std::cout << " " << id;
}

void VariableNode::printTreePrefix() {
  this->printTree();
}

void BlockNode::printTree() {
  for (Node* n : nodeList) {
    n->printTree();
    if (n->_type() != BASIC) {
      std::cout << std::endl;
    }
  }
}

void BlockNode::printTreePrefix() {
  for (Node* n : nodeList) {
    n->printTreePrefix();
    if (n->_type() != BASIC) {
      std::cout << std::endl;
    }
  }
}

void MessageNode::printTree() {
  std::cout << msg << " var:";
  node->printTree();
}

void MessageNode::printTreePrefix() {
  this->printTree();
}

void IfNode::printTree() {
  std::cout << "if:";
  condition->printTree();
  _then->printTree();
  if (_else != NULL) {
    _else->printTree();
  }
}

void IfNode::printTreePrefix() {
  this->printTree();
}

void CondNode::printTree() {
  boolExpr->printTreePrefix();
}

void CondNode::printTreePrefix() {
  this->printTree();
}

void ThenNode::printTree() {
  std::cout << std::endl;
  std::cout << "then:" << std::endl;
  lines->printTreePrefix();
}

void ThenNode::printTreePrefix() {
  this->printTree();
}

void ElseNode::printTree() {
  std::cout << "else:" << std::endl;
  lines->printTreePrefix();
}

void ElseNode::printTreePrefix() {
  this->printTree();
}

void ForNode::printTree() {
  this->printTreePrefix();
}

void ForNode::printTreePrefix() {
  std::cout << "for: ";
  this->assignNode->printTreePrefix();

  std::cout << ",";
  this->testNode->printTreePrefix();

  std::cout << ",";
  if (itNode->_type() != BASIC) {
    std::cout << " ";
  }
  this->itNode->printTreePrefix();

  std::cout << std::endl << "do:" << std::endl;
  this->doNode->printTreePrefix();
}
