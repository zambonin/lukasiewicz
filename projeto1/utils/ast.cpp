#include "ast.h"
#include "st.h"

using namespace AST;

extern ST::SymbolTable symbolTable;

void IntNode::printTree() {
  std::cout << value;
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
  }
  right->printTree();
}

void VariableNode::printTree() {
  if (next != NULL) {
    next->printTree();
    std::cout << ", ";
  }
  std::cout << id;
}

void BlockNode::printTree() {
  for (Node* n : nodeList) {
    n->printTree();
    std::cout << std::endl;
  }
}

int IntNode::computeTree() {
  return value;
}

int BinaryOpNode::computeTree() {
  int value, leftValue, rightValue;
  leftValue = left->computeTree();
  rightValue = right->computeTree();

  switch(binOp) {
    case add:
      value = leftValue + rightValue;
      break;
    case sub:
      value = leftValue - rightValue;
      break;
    case mul:
      value = leftValue * rightValue;
      break;
    case divs:
      value = leftValue / rightValue;
      break;
    case assign:
      VariableNode* leftVar = dynamic_cast<VariableNode*>(left); // ??
      symbolTable.entryList[leftVar->id].value = rightValue;
      value = rightValue;
  }

  return value;
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
