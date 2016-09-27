#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

BinaryOpNode::BinaryOpNode(Operation binOp, Node* left, Node* right):
binOp(binOp), left(left), right(right) {
  if (left->_type() != right->_type()) {
    errorMessage(binOp, left, right);
  }
}

NodeType BinaryOpNode::_type() {
  if (binOp == add || binOp == sub || binOp == mul || binOp == div) {
    return left->_type();
  } else {
    return BOOL;
  }
}

AssignmentNode::AssignmentNode(Node* left, Node* right):
binOp(assign), left(left), right(right) {
  if (left->_type() != right->_type()) {
    errorMessage(binOp, left, right);
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
    std::cout << std::endl;
  }
}

void BlockNode::printTreePrefix() {
  for (Node* n : nodeList) {
    n->printTreePrefix();
    std::cout << std::endl;
  }
}

void MessageNode::printTree() {
  std::cout << msg << " var:";
  node->printTree();
}

void MessageNode::printTreePrefix() {
  this->printTree();
}