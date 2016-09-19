/*
 *  Abstract syntax tree definition for a language
 *  called Łukasiewicz, based on prefix notation.
 *
 *  Authors: Douglas Martins, Gustavo Zambonin,
 *           Marcello Klingelfus
 */
#pragma once

#include <iostream>
#include <vector>

extern void yyerror(const char *s, ...);

namespace AST {

  // Enumeration of binary operations supported by the AST.
  enum Operation { add, sub, mul, divs, assign };
  // uminus / parenthesis as operator?

  // Generic node class for the AST.
  class Node {
  public:
    // Destructor for the Node class.
    virtual ~Node() {}

    // Prints contents of the tree whose root
    // is this node.
    virtual void printTree() {}

    // Computes the final result of the expression
    // represented by the tree whose root is this node.
    virtual int computeTree() {
      return 0;
    }
  };

  // Class for nodes that contain an integer.
  class IntNode : public Node {
  public:
    // Value of the node.
    int value;

    // Constructor for an integer node.
    IntNode(int value):
      value(value) {}

    // Prints contents of the tree whose root
    // is this node.
    void printTree();

    // Computes the final result of the expression
    // represented by the tree whose root is this node.
    int computeTree();
  };

  // Class for nodes that are binary operations.
  class BinaryOpNode : public Node {
  public:
    // Usual numeric operation.
    Operation binOp;

    // This node's left child.
    Node *left;

    // This node's right child.
    Node *right;

    // Constructor for a binary operation node.
    BinaryOpNode(Operation binOp, Node *left, Node *right):
      binOp(binOp), left(left), right(right) {}

    // Prints contents of the tree whose root
    // is this node.
    void printTree();

    // Computes the final result of the expression
    // represented by the tree whose root is this node.
    int computeTree();
  };

  // Class for nodes that are variables.
  class VariableNode : public Node {
  public:
    // Unique name for the variable.
    std::string id;

    // ??
    Node *next;

    // Constructor for the Variable node.
    VariableNode(std::string id, Node *next):
      id(id), next(next) {}

    // Prints contents of the tree whose root
    // is this node.
    void printTree();

    // Computes the final result of the expression
    // represented by the tree whose root is this node.
    int computeTree();
  };

  // Class that represents a line of the program.
  class BlockNode : public Node {
  public:
    // List of nodes representing the line tree.
    std::vector<Node*> nodeList;

    // Constructor for a block node.
    BlockNode() {}

    // Prints contents of the tree whose root
    // is this node.
    void printTree();

    // Computes the final result of the expression
    // represented by the tree whose root is this node.
    int computeTree();
  };

}
