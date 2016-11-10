/*!
 * Abstract syntax tree definition for a language
 * called Łukasiewicz, based on prefix notation.
 *
 *  \author Douglas Martins, Gustavo Zambonin, Marcello Klingelfus
 */
#pragma once

#include <deque>
#include <iostream>
#include <vector>

namespace AST {

  //! Operations accepted by the language.
  enum Operation {
    add, sub, mul, div, assign, index, addr, ref,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, _not, cast_int, cast_float, cast_bool
  };

  /*
   * Possible types for the nodes. This enum may overflow with multiple
   * references; it can be considered that any variable that has a `type`
   * value greater than 6 is a pointer, and `type` modulo 6 greater than
   * three is an array.
   */
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
    " -u", " !", " [int]", " [float]", " [bool]"
  };

  //! Verbose representation for the operations.
  static const std::string _opt[] = {
    "addition", "subtraction", "multiplication", "division", "attribution",
    "index", "address", "reference", "equal", "different", "greater than",
    "less than", "greater or equal than", "less or equal than", "and", "or",
    "unary minus", "negation"
  };

  //! Verbose representation for the node types.
  static const std::string _usr[] = { "integer", "float", "boolean" };

  //! Basic representation for the node types.
  static const std::string _var[] = { "int", "float", "bool" };

  class Node {
  public:
    //! Type of the node.
    NodeType type;

    //! Default constructor declared to prevent errors.
    Node();

    //! Basic constructor that also sets the type of the node.
    explicit Node(int type);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    virtual void print(bool /*prefix*/) {}

    //! Returns the type of the node.
    virtual NodeType _type() { return this->type; }

    //! Basic destructor.
    virtual ~Node() = default;
  };

  class IntNode : public Node {
  public:
    //! Integer value of the node.
    int value;

    //! Basic constructor that also sets the type of the node.
    explicit IntNode(int value): Node(0), value(value) {}

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;
  };

  class FloatNode : public Node {
  public:
    //! Char pointer value of the node, displaying exactly the user input.
    char* value;

    //! Basic constructor that also sets the type of the node.
    explicit FloatNode(char* value): Node(1), value(value) {}

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Basic destructor.
    ~FloatNode() override;
  };

  class BoolNode : public Node {
  public:
    //! Boolean value of the node.
    bool value;

    //! Basic constructor that also sets the type of the node.
    explicit BoolNode(bool value): Node(2), value(value) {}

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;
  };

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
    void print(bool prefix) override;

    //! Returns the type of the left child if the operation is arithmetic
    //! or an assignment, and a boolean type otherwise.
    NodeType _type() override;

    //! Basic destructor.
    ~BinaryOpNode() override;
  };

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
    void print(bool prefix) override;

    //! Basic destructor.
    ~UnaryOpNode() override;
  };

  class LinkedNode : public Node {
  public:
    //! Head of the linked list.
    Node* next;

    //! Basic constructor.
    LinkedNode(Node* next, int type): Node(type), next(next) {}

    //! Basic destructor.
    ~LinkedNode() override;
  };

  //! Represents a variable that may be simple or an array/pointer.
  class VariableNode : public LinkedNode {
  public:
    //! Name of the variable.
    char* id;

    //! Length of the array if applicable.
    int size;

    //! Basic constructor.
    VariableNode(char* id, Node* next, int type, int size):
    LinkedNode(next, type), id(id), size(size) {}

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Basic destructor.
    ~VariableNode() override;
  };

  class BlockNode : public Node {
  public:
    //! List of nodes that can be seen as lines of the program.
    std::vector<Node*> nodeList;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Basic destructor.
    ~BlockNode() override;
  };

  class MessageNode : public LinkedNode {
  public:
    //! Basic constructor.
    using LinkedNode::LinkedNode;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;
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
    void print(bool prefix) override;

    //! Basic destructor.
    ~IfNode() override;
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
    void print(bool prefix) override;

    //! Basic destructor.
    ~ForNode() override;
  };

  class FuncNode : public Node {
  public:
    //! Name of the function.
    char* id;

    //! Pointer to the head node of the parameter list,
    //! producing a data structure similar to a linked list.
    Node* params;

    //! Body of the function.
    BlockNode* contents;

    //! Basic constructor.
    FuncNode(char* id, Node* params, int type, BlockNode* contents);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Compares two linked lists of parameters to check if they
    //! contain the same nodes.
    /*!
     *  \param n      list to be compared against.
     */
    bool verifyParams(Node* n);

    //! Basic destructor.
    ~FuncNode() override;
  };

  class ParamNode : public VariableNode {
  public:
    //! Basic constructor.
    using VariableNode::VariableNode;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Produces a double ended queue with all the nodes on the
    //! linked list of parameters.
    std::deque<ParamNode*> createDeque();
  };

  class ReturnNode : public LinkedNode {
  public:
    //! Basic constructor.
    using LinkedNode::LinkedNode;

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;
  };

  class FuncCallNode : public Node {
  public:
    //! Name of the function being called.
    char* id;

    //! Node representing the original function.
    Node* function;

    //! List of parameters used in the function call.
    BlockNode* params;

    //! Basic constructor.
    FuncCallNode(char* id, Node* function, BlockNode* params);

    //! Prints the node contents to `stdout`.
    /*!
     *  \param prefix  chooses between polish or infix notation.
     */
    void print(bool prefix) override;

    //! Basic destructor.
    ~FuncCallNode() override;
  };

  //! Pretty-prints an object with `cout`. Useful for tabulation.
  /*!
   *  \param text     text to be printed.
   *  \param n        spaces to be added before the text.
   */
  template<typename T>
  void text(const T& text, int n);

  //! Prints the verbose type of the node, taking in account its
  //! status as an array and/or pointer.
  /*!
   *  \param node     node in question.
   *  \param _short   prints a short version of the type used in declarations.
   */
  std::string verboseType(Node* node, bool _short);

} // namespace AST
