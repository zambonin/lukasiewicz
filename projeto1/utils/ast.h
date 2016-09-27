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
    uminus, _not,
  };

  enum NodeType {
    INT, FLOAT, BOOL
  };

  // Generic node class for the AST.
  class Node {
  public:
    std::map<Operation, std::string> errorMsg = {
      {add, "addition"}, {sub, "subtraction"}, {mul, "multiplication"},
      {div, "division"}, {assign, "attribution"}, {eq, "equal"},
      {neq, "different"}, {gt, "greater than"}, {lt, "less than"},
      {geq, "greater or equal than"}, {leq, "less or equal than"},
      {_and, "and"}, {_or, "or"}, {uminus, "unary minus"},
      {_not, "negation"},
    };

    std::map<std::string, NodeType> nodeTypeString = {
        {"integer", INT}, {"float", FLOAT}, {"boolean", BOOL}
    };

    std::map<NodeType, std::string> nodeName = {
      {INT, "integer"}, {FLOAT, "float"}, {BOOL, "boolean"},
    };

    virtual void printTree() {}
    virtual void printTreePrefix() {}
    virtual NodeType _type() { return INT; }

    void errorMessage(Operation op, Node* n1, Node* n2) {
      yyerror("semantic error: %s operation expected %s but received %s\n",
              errorMsg[op].c_str(), nodeName[n1->_type()].c_str(),
              nodeName[n2->_type()].c_str());
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

    void printTree() { std::cout << " " << value; }
    void printTreePrefix() { std::cout << " " << value; }
    NodeType _type() { return INT; }

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
    NodeType _type() { return FLOAT; }

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
    NodeType _type() { return BOOL; }

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
    BinaryOpNode(Operation binOp, Node* left, Node* right);

    void printTree();
    void printTreePrefix();
    NodeType _type();

  };

  class UnaryOpNode : public Node {
  public:
    // Usual numeric operation.
    Operation op;

    Node* node;

    std::map<Operation, std::string> strOp = {
      {uminus, " -u"}, {_not, " !"},
    };

    std::map<Operation, std::string> errorMsg = {
    };

    // Constructor for a binary operation node.
    UnaryOpNode(Operation op, Node* node):
    op(op), node(node) {}

    void printTree();
    void printTreePrefix();
    NodeType _type() { return node->_type(); }

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
    AssignmentNode(Node* left, Node* right);

    void printTree();
    void printTreePrefix();
    NodeType _type() { return left->_type(); }

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
    NodeType _type();

  };

  // Class that represents a line of the program.
  class BlockNode : public Node {
  public:
    // List of nodes representing the line tree.
    std::vector<Node*> nodeList;

    void printTree();
    void printTreePrefix();
    NodeType _type() { return INT; }

  };

  class MessageNode : public Node {
  public:

    Node* node;
    std::string msg;

    MessageNode(Node* node, std::string msg):
    node(node), msg(msg) {}

    void printTree();
    void printTreePrefix();
    NodeType _type() { return node->_type(); }

  };

 }
