#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

void IntNode::printTree() {
  std::cout << value << "";
}

void IntNode::printTreePrefix() {
   std::cout << value << " ";
}
void BinaryOpNode::printTree() {
  left->printTree();
  switch(binOp) {
    case add:
      std::cout << " + ";
      break;
    case sub:
      std::cout << " - ";
      break;
    case mul:
      std::cout << " * ";
      break;
    case divs:
      std::cout << " / ";
      break;
    case assign:
      std::cout << " = ";
      break;
    case uminus:
      std::cout << " -u ";
      break;
  }
  right->printTree();
}

void BinaryOpNode::printTreePrefix() {
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
    case divs:
      std::cout << "/ ";
      break;
    case assign:
      std::cout << "= ";
      break;
    case uminus:
      std::cout << "-u ";
      break;
  }
  left->printTreePrefix();
  right->printTreePrefix();
}

void AssignmentNode::printTree() {
  left->printTree();
  std::cout << " = ";
  right->printTree();
}

void AssignmentNode::printTreePrefix() {
  left->printTree();
  std::cout << " = ";
  right->printTree();
}


void VariableNode::printTree() {
  if (next != NULL) {
    next->printTree();
    std::cout << ", ";
  }
  std::cout << id;
}

void VariableNode::printTreePrefix() {
    if (next != NULL) {
      next->printTree();
      std::cout << ", ";
    }
  std::cout << id;
}

void BlockNode::printPrefix() {
  for (Node *n : nodeList){
    n->printTreePrefix();
    std::cout << std::endl;
  }
}

void BlockNode::printTree() {
  for (Node* n : nodeList) {
    n->printTree();
    std::cout << std::endl;
  }
}

void BlockAssignmentNode::printTree() {
  std::cout << "var " << type << ": ";
  for (Node *n : nodeList){
    n->printTreePrefix();
    std::cout << std::endl;
  }
}

void BlockAssignmentNode::printPrefix() {
  std::cout << "var " << type << ": ";
  for (Node *n : nodeList){
    n->printTreePrefix();
    std::cout << std::endl;
  }
}
