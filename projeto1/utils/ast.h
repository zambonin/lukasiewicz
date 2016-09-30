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

namespace AST {

  enum Operation {
    add, sub, mul, div, assign,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, _not, cast_int, cast_float, cast_bool, _if
  };

  enum NodeType {
    BASIC, INT, FLOAT, BOOL
  };

  class Node {
  public:
    NodeType type;

    std::map<Operation, std::string> errorMsg = {
      {add, "addition"}, {sub, "subtraction"}, {mul, "multiplication"},
      {div, "division"}, {assign, "attribution"}, {eq, "equal"},
      {neq, "different"}, {gt, "greater than"}, {lt, "less than"},
      {geq, "greater or equal than"}, {leq, "less or equal than"},
      {_and, "and"}, {_or, "or"}, {uminus, "unary minus"},
      {_not, "negation"}, {_if, "test"},
    };

    std::map<Operation, std::string> strOp = {
      {add, "+"}, {sub, "-"}, {mul, "*"}, {div, "/"}, {assign, "="},
      {eq, "=="}, {neq, "!="}, {gt, ">"}, {lt, "<"},
      {geq, ">="}, {leq, "<="}, {_and, "&"}, {_or, "|"},
      {uminus, " -u"}, {_not, " !"}, {cast_int, " [int]"},
      {cast_float, " [float]"}, {cast_bool, " [bool]"},
    };

    std::map<std::string, NodeType> nodeTypeString = {
      {"integer", INT}, {"float", FLOAT}, {"boolean", BOOL},
    };

    std::map<NodeType, std::string> nodeName = {
      {INT, "integer"}, {FLOAT, "float"}, {BOOL, "boolean"},
    };

    Node() {}

    virtual void print(bool prefix) {}
    virtual NodeType _type() { return BASIC; }
    void errorMessage(Operation op, Node* n1, Node* n2);

  };

  class IntNode : public Node {
  public:
    int value;

    IntNode(int value);

    void print(bool prefix);
    NodeType _type();

  };

  class FloatNode : public Node {
  public:
    char* value;

    FloatNode(char* value);

    void print(bool prefix);
    NodeType _type();

  };

  class BoolNode : public Node {
  public:
    bool value;

    BoolNode(bool value);

    void print(bool prefix);
    NodeType _type();

  };

  class BinaryOpNode : public Node {
  public:
    Operation binOp;
    Node* left;
    Node* right;

    BinaryOpNode(Operation binOp, Node* left, Node* right);

    void print(bool prefix);
    NodeType _type();

  };

  class UnaryOpNode : public Node {
  public:
    Operation op;
    Node* node;

    UnaryOpNode(Operation op, Node* node);

    void print(bool prefix);
    NodeType _type();

  };

  class VariableNode : public Node {
  public:
    std::string id;
    Node* next;

    VariableNode(std::string id, Node* next):
    id(id), next(next) {}

    void print(bool prefix);
    NodeType _type();

  };

  class BlockNode : public Node {
  public:
    std::vector<Node*> nodeList;

    void print(bool prefix);

  };

  class MessageNode : public Node {
  public:
    Node* node;

    MessageNode(Node* node):
    node(node) {}

    void print(bool prefix);
    NodeType _type();

  };

  class IfNode : public Node {
  public:
    Node* condition;
    BlockNode* _then;
    BlockNode* _else;

    IfNode(Node* condition, BlockNode* _then, BlockNode* _else);

    void print(bool prefix);

  };

  class ForNode : public Node {
  public:
    Node* assignNode;
    Node* testNode;
    Node* itNode;
    Node* doNode;

    ForNode(Node* assignNode, Node* testNode, Node* itNode, Node* doNode):
    assignNode(assignNode), testNode(testNode), itNode(itNode), doNode(doNode) {}

    void print(bool prefix);

  };

}
