/*!
 * Abstract syntax tree definition for a language
 * called Łukasiewicz, based on prefix notation.
 *
 *  \author Douglas Martins, Gustavo Zambonin, Marcello Klingelfus
 */
#pragma once

#include <deque>
#include <iostream>
#include <sstream>
#include <vector>

namespace AST {

  /* Macros that reduce the visual pollution when indentation is needed. */

  /* Takes a single line of code and indents it with two spaces. */
  #define _tab(X)     spaces += 2; (X); spaces -= 2

  /* Variadic macro that prevents indentation for any number of lines. */
  #define _notab(...) int tmp = spaces; spaces = 0; (__VA_ARGS__); spaces = tmp;

  /* Checks if a node is an array. */
  #define notArray(X) (((X)->_type() % 8) < 4)

  /* Saves the current indentation status. */
  static int spaces;

  //! Operations accepted by the language.
  enum Operation {
    add, sub, mul, div, assign, index, addr, ref,
    eq, neq, gt, lt, geq, leq, _and, _or,
    uminus, _not, cast_int, cast_float,
    cast_bool, cast_word, len, append
  };

  /*
   * Possible types for the nodes. This enum may overflow with multiple
   * references; it can be considered that any variable that has a `type`
   * value greater than 8 is a pointer, and `type` modulo 8 greater than
   * four is an array.
   */
  enum NodeType {
    ND = -1,
    INT,    FLOAT,    BOOL,     CHAR,
    A_INT,  A_FLOAT,  A_BOOL,   A_CHAR,
    P_INT,  P_FLOAT,  P_BOOL,   P_CHAR,
    PA_INT, PA_FLOAT, PA_BOOL,  PA_CHAR,
  };

  //! Verbose representation for the operations.
  static const std::string _opt[] = {
    "addition", "subtraction", "multiplication", "division", "attribution",
    "index", "address", "reference", "equal", "different", "greater than",
    "less than", "greater or equal than", "less or equal than", "and", "or",
    "unary minus", "negation", "length", "append"
  };

  //! Verbose representation for the node types.
  static const std::string _usr[] = {
    "integer", "float", "boolean", "character"
  };

  //! Basic representation for the node types.
  static const std::string _var[] = { "int", "float", "bool", "char" };

  class Node {
  public:
    //! Type of the node.
    NodeType type;

    //! Default constructor declared to prevent errors.
    Node();

    //! Basic constructor that also sets the type of the node.
    explicit Node(int type);

    //! Prints the verbose type of the node, taking in account its
    //! status as an array and/or pointer.
    /*!
     *  \param node     node in question.
     *  \param _short   prints a short version of the type used in declarations.
     */
    std::string _vtype(bool _short);

    // todo doc
    virtual void printInfix() {}
    virtual void printPrefix() { this->printInfix(); }
    virtual void printPython() {}

    virtual void error_handler() {}

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

    void printInfix() override;
    void printPython() override;
  };

  class FloatNode : public Node {
  public:
    //! String value of the node, displaying exactly the user input.
    std::string value;

    //! Basic constructor that also sets the type of the node.
    explicit FloatNode(std::string value): Node(1), value(value) {}

    void printInfix() override;
    void printPython() override;
  };

  class BoolNode : public Node {
  public:
    //! Boolean value of the node.
    bool value;

    //! Basic constructor that also sets the type of the node.
    explicit BoolNode(bool value): Node(2), value(value) {}

    void printInfix() override;
    void printPython() override;
  };

 class CharNode : public Node {
  public:
    //! String value of the node, displaying exactly the user input.
    std::string value;

    //! Basic constructor that also sets the type of the node.
    explicit CharNode(std::string value): Node(3), value(value) {}

    void printInfix() override;
    void printPython() override;

    //! Returns a char array if the word starts with double quotes.
    NodeType _type() override;
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

    void printInfix() override;
    void printPrefix() override;
    void printPython() override;

    void error_handler() override;

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

    void printInfix() override;
    void printPrefix() override;
    void printPython() override;

    void error_handler() override;

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
    std::string id;

    //! Length of the array if applicable.
    int size;

    //! Checks if the variable is initialized.
    bool init;

    //! Basic constructor.
    VariableNode(std::string id, Node* next, int type, int size):
    LinkedNode(next, type), id(id), size(size) {}

    void printInfix() override;
    void printPython() override;
  };

  class BlockNode : public Node {
  public:
    //! List of nodes that can be seen as lines of the program.
    std::vector<Node*> nodeList;

    //! Default constructor.
    BlockNode() {}

    //! Basic constructor that pushes `n` to `nodeList`.
    BlockNode(Node* n);

    void printPrefix() override;
    void printPython() override;

    //! Basic destructor.
    ~BlockNode() override;
  };

  class MessageNode : public LinkedNode {
  public:
    //! Basic constructor.
    using LinkedNode::LinkedNode;

    void printPrefix() override;
    void printPython() override;
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

    void printPrefix() override;
    void printPython() override;

    void error_handler() override;

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

    void printPrefix() override;
    void printPython() override;

    void error_handler() override;

    //! Basic destructor.
    ~ForNode() override;
  };

  class FuncNode : public Node {
  public:
    //! Name of the function.
    std::string id;

    //! Pointer to the head node of the parameter list,
    //! producing a data structure similar to a linked list.
    Node* params;

    //! Body of the function.
    BlockNode* contents;

    //! Basic constructor.
    FuncNode(std::string id, Node* params, int type, BlockNode* contents);

    void printPrefix() override;
    void printPython() override;

    //! Compares two linked lists of parameters to check if they
    //! contain the same nodes.
    /*!
     *  \param n      list to be compared against.
     */
    bool verifyParams(Node* n);

    //! Produces a double ended queue with all the nodes on the
    //! linked list of parameters.
    std::deque<VariableNode*> createDeque();

    void error_handler() override;

    //! Basic destructor.
    ~FuncNode() override;
  };

  class ParamNode : public VariableNode {
  public:
    //! Basic constructor.
    using VariableNode::VariableNode;

    void printInfix() override;
    void printPython() override;
  };

  class ReturnNode : public LinkedNode {
  public:
    //! Basic constructor.
    ReturnNode(Node* next): LinkedNode(next, next->_type()) {}

    void printPython() override;
    void printPrefix() override;
  };

  class FuncCallNode : public Node {
  public:
    //! Node representing the original function.
    FuncNode* function;

    //! List of parameters used in the function call.
    BlockNode* params;

    //! Basic constructor.
    FuncCallNode(FuncNode* function, BlockNode* params);

    void printPython() override;
    void printPrefix() override;

    void error_handler() override;

    //! Returns the type of the original function.
    NodeType _type() override;

    //! Basic destructor.
    ~FuncCallNode() override;
  };

  class DeclarationNode : public VariableNode {
  public:
    //! Basic constructor.
    using VariableNode::VariableNode;

    void printInfix() override;
    void printPython() override;
  };

  class HiOrdFuncNode : public FuncNode {
  public:
    //! Basic constructor.
    HiOrdFuncNode(std::string id, Node* func, VariableNode* array);

    virtual void hi_error_handler(Node*);

    //! Returns the appropriate subclass given the id.
    static HiOrdFuncNode* chooseFunc(
      std::string id, Node* func, VariableNode* array);
  };

  class MapFuncNode : public HiOrdFuncNode {
  public:
    //! Basic constructor.
    MapFuncNode(std::string id, Node* func, VariableNode* array);

    void hi_error_handler(Node*) override;
  };

  class FoldFuncNode : public HiOrdFuncNode {
  public:
    //! Basic constructor.
    FoldFuncNode(std::string id, Node* func, VariableNode* array);

    void hi_error_handler(Node*) override;
  };

  class FilterFuncNode : public HiOrdFuncNode {
  public:
    //! Basic constructor.
    FilterFuncNode(std::string id, Node* func, VariableNode* array);

    void hi_error_handler(Node*) override;
  };

  //! Pretty-prints an object with `cout`. Useful for tabulation.
  /*!
   *  \param text     text to be printed.
   *  \param n        spaces to be added before the text.
   */
  template<typename T>
  void text(const T& text, int n) {
    std::string blank(n, ' ');
    std::cout << blank << text;
  }

} // namespace AST

extern AST::BlockNode* string_read(const char* s);
extern void yyerror(const char* s, ...);
extern void yyserror(const char* s, ...);
