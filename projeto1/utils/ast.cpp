#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

void IntNode::printTree() {
  std::cout << " " << value;
}

void IntNode::printTreePrefix() {
  std::cout << " " << value;
}

void BinaryOpNode::printTree() {
  left->printTree();
  switch(binOp) {
    case add:
    std::cout << "+ ";
    break;
    case sub:
    std::cout << "- ";
    break;
    case mul:
    std::cout << "* ";
    break;
    case div:
    std::cout << "/ ";
    break;
    case assign:
    std::cout << "=";
    break;
    default:
    break;
  }
  right->printTree();
}

void BinaryOpNode::printTreePrefix() {
  switch(binOp) {
    case add:
    std::cout << " +";
    break;
    case sub:
    std::cout << " -";
    break;
    case mul:
    std::cout << " *";
    break;
    case div:
    std::cout << " /";
    break;
    case assign:
    std::cout << "=";
    break;
    default:
    break;
  }
  left->printTreePrefix();
  right->printTreePrefix();
}

void UnaryOpNode::printTree() {
  switch(op) {
    case uminus:
    std::cout << " -u";
    break;
    default:
    break;
  }
  node->printTree();
}

void UnaryOpNode::printTreePrefix() {
  switch(op) {
    case uminus:
    std::cout << " -u";
    break;
    default:
    break;
  }
  node->printTreePrefix();
}

void AssignmentNode::printTree() {
  left->printTree();
  std::cout << " =";
  right->printTree();
}

void AssignmentNode::printTreePrefix() {
  left->printTree();
  std::cout << " =";
  right->printTree();
}

void VariableNode::printTree() {
  if (next != NULL) {
    next->printTree();
    std::cout << ",";
  }
  std::cout << " " << id;
}

void VariableNode::printTreePrefix() {
  if (next != NULL) {
    next->printTree();
    std::cout << ",";
  }
  std::cout << " " << id;
}

void BlockNode::printTree() {
  for (Node* n : nodeList) {
    n->printTree();
    std::cout << std::endl;
  }
}

void BlockNode::printTreePrefix() {
  for (Node *n : nodeList){
    n->printTreePrefix();
    std::cout << std::endl;
  }
}

void MessageNode::printTree() {
  std::cout << msg << " var:";
  node->printTree();
}

void MessageNode::printTreePrefix() {
  std::cout << msg << " var:";
  node->printTreePrefix();
}
