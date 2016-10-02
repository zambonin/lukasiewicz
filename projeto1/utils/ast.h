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

namespace AST {

  enum Operation {
    add, sub, mul, div, assign,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, _not, cast_int, cast_float, cast_bool,
    if_test
  };

  enum NodeType {
    INT, FLOAT, BOOL, ND
  };

  static const std::string _bin[] = {
    "+", "-", "*", "/", "=",
    "==", "!=", ">", "<", ">=", "<=", "&", "|",
    " -u", " !",
    " [int]", " [float]", " [bool]"
  };

  static const std::string _opt[] = {
    "addition", "subtraction", "multiplication", "division", "attribution",
    "equal",  "different", "greater than", "less than",
    "greater or equal than", "less or equal than", "and", "or",
    "unary minus", "negation", "test"
  };

  static const std::string _usr[] = { "integer", "float", "boolean" };
  static const std::string _var[] = { "int", "float", "bool" };

  class Node {
  public:
    NodeType type;

    Node() {}

    virtual void print(bool prefix) {}
    virtual NodeType _type() { return ND; }
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
    NodeType type;

    VariableNode(std::string id, Node* next, NodeType type):
    id(id), next(next), type(type) {}

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
    Node* assign;
    Node* test;
    Node* iteration;
    BlockNode* body;

    ForNode(Node* assign, Node* test, Node* iteration, BlockNode* body):
    assign(assign), test(test), iteration(iteration), body(body) {}

    void print(bool prefix);

  };

}
