#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

void IntNode::printTree() {
  std::cout << value << "\n";
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

void VariableNode::printTree() {
  if (next != NULL) {
    next->printTree();
    std::cout << ", ";
  }
  std::cout << id;
}

void VariableNode::printTreePrefix() {
  if (next != NULL) {
    next->printTreePrefix();
    std::cout << ", ";
  }
  std::cout << id << " ";
}

void BlockNode::printTree() {
  for (Node* n : nodeList) {
    n->printTree();
    std::cout << std::endl;
  }
}

void BlockNode::printPrefix() {
  for (Node *n : nodeList){
    n->printTreePrefix();
    std::cout << std::endl;
  }
}

int IntNode::computeTree() {
  return value;
}

int BinaryOpNode::computeTree() {
  return 0;
}

int VariableNode::computeTree() {
  return symbolTable.entryList[id].value;
}

int BlockNode::computeTree() {
  int value;

  for (Node* n : nodeList) {
    value = n->computeTree();
    std::cout << "Computed " << value << std::endl;
  }

  return 0;
}
