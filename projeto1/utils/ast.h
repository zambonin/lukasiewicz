/*
 *  Abstract syntax tree definition for a language
 *  called Łukasiewicz, based on prefix notation.
 *
 *  Authors: Douglas Martins, Gustavo Zambonin,
 *           Marcello Klingelfus
 */
#pragma once

#include <iostream>
#include <map>
#include <vector>

 extern void yyerror(const char* s, ...);

 namespace AST {

  // Enumeration of operations supported by the AST.
  enum Operation {
    add, sub, mul, div, assign,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, negation
  };

  // Generic node class for the AST.
  class Node {
  public:
    virtual void printTree() {}
    virtual void printTreePrefix() {}
  };

  // Class for nodes that contain an integer.
  class IntNode : public Node {
  public:
    // Value of the node.
    int value;

    // Constructor for an integer node.
    IntNode(int value):
    value(value) {}

    void printTree() { std::cout << " " << value; }
    void printTreePrefix() { std::cout << " " << value; }

  };

  class FloatNode : public Node {
  public:
    // Value of the node.
    char* value;

    // Constructor for an integer node.
    FloatNode(char* value):
    value(value) {}

    void printTree() { std::cout << " " << value; }
    void printTreePrefix() { std::cout << " " << value; }

  };

  class BoolNode : public Node {
  public:
    // Value of the node.
    bool value;

    std::string strValue;

    // Constructor for an integer node.
    BoolNode(bool value):
    value(value) { strValue = value ? "true" : "false"; }

    void printTree() { std::cout << " " << strValue; }
    void printTreePrefix() { std::cout << " " << strValue; }

  };

  // Class for nodes that are binary operations.
  class BinaryOpNode : public Node {
  public:
    // Usual numeric operation.
    Operation binOp;

    // This node's children.
    Node* left;
    Node* right;

    std::map<Operation, std::string> strOp = {
      {add, "+"}, {sub, "-"}, {mul, "*"}, {div, "/"}, {assign, "="},
      {eq, "=="}, {neq, "!="}, {gt, ">"}, {lt, "<"},
      {geq, ">="}, {leq, "<="}, {_and, "&"}, {_or, "|"},
    };

    // Constructor for a binary operation node.
    BinaryOpNode(Operation binOp, Node* left, Node* right):
    binOp(binOp), left(left), right(right) {}

    void printTree();
    void printTreePrefix();

  };

  class UnaryOpNode : public Node {
  public:
    // Usual numeric operation.
    Operation op;

    Node* node;

    std::map<Operation, std::string> strOp = {
      {uminus, " -u"}, {negation, " !"},
    };

    // Constructor for a binary operation node.
    UnaryOpNode(Operation op, Node* node):
    op(op), node(node) {}

    void printTree();
    void printTreePrefix();

  };

  // Class for nodes that are binary operations.
  class AssignmentNode : public Node {
  public:
    // Usual numeric operation.
    Operation binOp;

    // This node's children.
    Node* left;
    Node* right;

    // Constructor for a binary operation node.
    AssignmentNode(Node* left, Node* right):
    binOp(AST::assign), left(left), right(right) {}

    void printTree();
    void printTreePrefix();

  };

  // Class for nodes that are variables.
  class VariableNode : public Node {
  public:
    // Unique name for the variable.
    std::string id;

    // Reference to next node when there are multiple assignments.
    Node* next;

    // Constructor for the Variable node.
    VariableNode(std::string id, Node* next):
    id(id), next(next) {}

    void printTree();
    void printTreePrefix();

  };

  // Class that represents a line of the program.
  class BlockNode : public Node {
  public:
    // List of nodes representing the line tree.
    std::vector<Node*> nodeList;

    void printTree();
    void printTreePrefix();

  };

  class MessageNode : public Node {
  public:

    Node* node;
    std::string msg;

    MessageNode(Node* node, std::string msg):
    node(node), msg(msg) {}

    void printTree();
    void printTreePrefix();

  };

 }
