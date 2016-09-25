#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

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
