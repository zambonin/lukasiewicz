/*!
 * Abstract syntax tree definition for a language
 * called Łukasiewicz, based on prefix notation.
 *
 *  \author Douglas Martins, Gustavo Zambonin, Marcello Klingelfus
 */
#pragma once

#include <iostream>
#include <vector>

namespace AST {

  //! Operations accepted by the language.
  enum Operation {
    add, sub, mul, div, assign, index, addr, ref,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, _not, if_test, cast_int, cast_float, cast_bool
  };

  //! Possible types for the nodes.
  enum NodeType {
    ND = -1, INT, FLOAT, BOOL,
    A_INT, A_FLOAT, A_BOOL,
    P_INT, P_FLOAT, P_BOOL,
    PA_INT, PA_FLOAT, PA_BOOL,
  };

  //! String representation for the operations.
  static const std::string _bin[] = {
    "+", "-", "*", "/", "=", "[index]", " [addr]", " [ref]",
    "==", "!=", ">", "<", ">=", "<=", "&", "|",
    " -u", " !", "", " [int]", " [float]", " [bool]"
  };

  //! Verbose representation for the operations.
  static const std::string _opt[] = {
    "addition", "subtraction", "multiplication", "division", "attribution",
    "index", "address", "reference", "equal", "different", "greater than",
    "less than", "greater or equal than", "less or equal than", "and", "or",
    "unary minus", "negation", "test"
  };

  //! Verbose representation for the node types.
  static const std::string _usr[] = { "integer", "float", "boolean" };

  //! Basic representation for the node types.
  static const std::string _var[] = { "int", "float", "bool" };

  //! Parent class representing a basic node.
  class Node {
  public:
    //! Type of the node.
    NodeType type;

    //! Reference counter.
    int ptr_cnt = 0;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    virtual void print(bool prefix) {}

    //! Returns the type of the node.
    virtual NodeType _type() { return this->type; }

    //! Prints a semantic error message.
    /*!
     *  \param op   operation of the node.
     *  \param n1   left child (operand) of the node.
     *  \param n2   right child (operand) of the node.
     */
    void errorMessage(Operation op, Node* n1, Node* n2);

    //! Prints the verbose type of the node, taking in account its
    //! status as an array and/or pointer.
    std::string verboseType();

    virtual ~Node() {}

  };

  //! Represents a node containing an integer.
  class IntNode : public Node {
  public:
    //! Integer value of the node.
    int value;

    //! Basic constructor that also sets the type of the node.
    IntNode(int value);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the node.
    NodeType _type();

  };

  //! Represents a node containing a floating point variable.
  class FloatNode : public Node {
  public:
    //! Char pointer value of the node, displaying exactly the user input.
    char* value;

    //! Basic constructor that also sets the type of the node.
    FloatNode(char* value);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the node.
    NodeType _type();

    ~FloatNode() {
      free(value);
    }

  };

  //! Represents a node containing a boolean variable.
  class BoolNode : public Node {
  public:
    //! Boolean value of the node.
    bool value;

    //! Basic constructor that also sets the type of the node.
    BoolNode(bool value);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the node.
    NodeType _type();

  };

  //! Node representing a binary operation, such as arithmetic or logic ones.
  class BinaryOpNode : public Node {
  public:
    //! Operation enum value of the node.
    Operation binOp;

    //! Left operand or left child of the node.
    Node* left;

    //! Right operand or right child of the node.
    Node* right;

    //! Basic constructor. Checks for semantic errors and enforces coercion.
    BinaryOpNode(Operation binOp, Node* left, Node* right);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the left child if the operation is arithmetic
    //! or an assignment, and a boolean type otherwise.
    NodeType _type();

    ~BinaryOpNode() {
      if (left != nullptr) delete left;
      if (right != nullptr) delete right;
    }

  };

  //! Node representing an unary operation, such as the unary minus or casting.
  class UnaryOpNode : public Node {
  public:
    //! Operation enum value of the node.
    Operation op;

    //! Only operand or child of the node.
    Node* node;

    //! Basic constructor that also sets the type of this node.
    UnaryOpNode(Operation op, Node* node);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of this node.
    NodeType _type();

    ~UnaryOpNode() {
      if (node != nullptr) delete node;
    }

  };

  class VariableNode : public Node {
  public:
    //! Name of the variable.
    char* id;

    //! Pointer to the next node in the case of multiple declarations on
    //! the same line, producing a data structure similar to a linked list.
    Node* next;

    //! Type of the node.
    NodeType type;

    //! Length of the array if applicable.
    int size;

    //! Basic constructor.
    VariableNode(char* id, Node* next, NodeType type, int size, int ref);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the node.
    NodeType _type();

    ~VariableNode() {
      free(id);
      if (next != nullptr) delete next;
    }

  };

  class BlockNode : public Node {
  public:
    //! List of nodes that can be seen as lines of the program.
    std::vector<Node*> nodeList;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    ~BlockNode() {
      for (auto n : nodeList) delete n;
    }
  };

  class MessageNode : public Node {
  public:
    //! Pointer to the last VariableNode of the line.
    Node* node;

    //! Type of the declaration.
    std::string msg;

    //! Basic constructor.
    MessageNode(Node* node, std::string msg, int ref);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    //! Returns the type of the node.
    NodeType _type();

    ~MessageNode() {
      if (node != nullptr) delete node;
    }

  };

  class IfNode : public Node {
  public:
    //! Pointer to the node representing the
    //! expression for the condition of the `if`.
    Node* condition;

    //! Pointer to the node representing the
    //! lines inside the `then` clause.
    BlockNode* _then;

    //! Pointer to the node representing the
    //! lines inside the `else` clause.
    BlockNode* _else;

    //! Basic constructor.
    IfNode(Node* condition, BlockNode* _then, BlockNode* _else);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    ~IfNode() {
      if (condition != nullptr) delete condition;
      if (_then != nullptr) delete _then;
      if (_else != nullptr) delete _else;
    }

  };

  class ForNode : public Node {
  public:
    //! Pointer to the node representing the first expression
    //! of the `for` node, generally an assignment.
    Node* assign;

    //! Pointer to the node representing the second expression
    //! of the `for` node, generally a logical operation.
    Node* test;

    //! Pointer to the node representing the third expression
    //! of the `for` node, generally a simple increment.
    Node* iteration;

    //! Pointer to the node representing the lines inside the `for`.
    BlockNode* body;

    //! Basic constructor.
    ForNode(Node* assign, Node* test, Node* iteration, BlockNode* body);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix);

    ~ForNode() {
      if (assign != nullptr) delete assign;
      if (test != nullptr) delete test;
      if (iteration != nullptr) delete iteration;
      if (body != nullptr) delete body;
    }

  };

}
