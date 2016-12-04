#include "ast.h"

namespace AST {

/* Verbose representation for the operations. */
static const std::string _opt[] = {
  "addition", "subtraction", "multiplication", "division", "attribution",
  "index", "address", "reference", "equal", "different", "greater than",
  "less than", "greater or equal than", "less or equal than", "and", "or",
  "unary minus", "negation", "length", "append"
};

void BinaryOpNode::error_handler() {
  VariableNode* v1 = dynamic_cast<VariableNode*>(left);
  VariableNode* v2 = dynamic_cast<VariableNode*>(right);
  FuncCallNode* f1 = dynamic_cast<FuncCallNode*>(right);
  unsigned int n = 0;

  if (v2 != nullptr) {
    n = v2->size;
  } else if (f1 != nullptr && !notArray(f1->function)) {
    n = dynamic_cast<VariableNode*>(dynamic_cast<ReturnNode*>(
      f1->function->contents->nodeList.back())->next)->size;
  }
  if (v1 != nullptr && (v1->size < n)) {
    yyserror("operation between mismatched array sizes");
  }

  if (left->_type() == A_CHAR && right->_type() == A_CHAR) {
    CharNode* c = dynamic_cast<CharNode*>(right);
    if (c != nullptr && v1 != nullptr && v1->size < c->value.size() - 2) {
      c->value.resize(v1->size + 1UL);
      c->value += R"(")";
      yyerror("warning: value truncated to %s", c->value.c_str());
    }
  }

  bool differentTypes = (left->_type() != right->_type());
  bool bothValid = (left->_type() >= 0 && right->_type() >= 0);

  if (binOp == index) {
    if (notArray(left)) {
      yyserror("left hand side of index operation is not an array");
    } else if (right->_type() != INT) {
      yyserror("index operation expected integer but received %s",
        right->_vtype(false).c_str());
    }
  } else if (binOp == append) {
    if (notArray(left)) {
      yyserror("left hand side of append operation is not an array");
    } else if ((left->_type() % 4) != right->_type()) {
      auto n = new Node(left->_type() % 4);
      yyserror("append operation expected %s but received %s",
        n->_vtype(false).c_str(), right->_vtype(false).c_str());
      delete n;
    } else {
      dynamic_cast<VariableNode*>(left)->size++;
    }
  } else if (differentTypes && bothValid) {
    yyserror("%s operation expected %s but received %s",
      _opt[binOp].c_str(), left->_vtype(false).c_str(),
      right->_vtype(false).c_str());
  }
}

void UnaryOpNode::error_handler() {
  if (op == ref && type < 0) {
    yyserror("reference operation expects a pointer");
  } else if (op == len && notArray(node)) {
    yyserror("length operation expects an array");
  } else if (op == addr) {
    bool isNotVar = (dynamic_cast<VariableNode*>(node) == nullptr);
    AST::BinaryOpNode* indexNode = dynamic_cast<BinaryOpNode*>(node);
    bool isNotIndex = (indexNode != nullptr && indexNode->binOp != index);
    if ((indexNode == nullptr && isNotVar) || isNotIndex) {
       yyserror("address operation expects a variable or array item");
     }
  }
}

void IfNode::error_handler() {
  if (condition->_type() != BOOL) {
    yyserror("test operation expected boolean but received %s",
      condition->_vtype(false).c_str());
  }
}

void ForNode::error_handler() {
  if (test->_type() != BOOL) {
    yyserror("test operation expected boolean but received %s",
      test->_vtype(false).c_str());
  }
}

void FuncNode::error_handler() {
  if (contents != nullptr) {
    Node* ret = contents->nodeList.back();
    bool isReturn = (dynamic_cast<ReturnNode*>(ret) != nullptr);
    if (type != ret->_type() && isReturn) {
      yyserror("function %s has incoherent return type", id.c_str());
    }
  }
}

void FuncCallNode::error_handler() {
  std::vector<Node*> callParam = params->nodeList;
  int callSize = callParam.size();

  std::deque<VariableNode*> origParam = function->createDeque();
  int origSize = origParam.size();

  if (origSize != callSize) {
    yyserror("function %s expects %d parameters but received %d",
      function->id.c_str(), origSize, callSize);
  } else {
    for (int i = 0; i < origSize; ++i) {
      if (origParam[i]->_type() != callParam[i]->_type()) {
        yyserror("parameter %s expected %s but received %s",
          origParam[i]->id.c_str(),
          origParam[i]->_vtype(false).c_str(),
          callParam[i]->_vtype(false).c_str());
      }
    }
  }
}

void HiOrdFuncNode::hi_error_handler(Node* array) {
  if (notArray(array)) {
    yyserror("high order function's second parameter must be of array type");
  }
}

void MapFuncNode::hi_error_handler(Node* func) {
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != (this->type - 4)) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 1) {
    yyserror("map's lambda expects 1 parameters but received %d", n);
  }
}

void FoldFuncNode::hi_error_handler(Node* func) {
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != this->type) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 2) {
    yyserror("fold's lambda expects 2 parameters but received %d", n);
  }
}

void FilterFuncNode::hi_error_handler(Node* func) {
  FuncNode* f = dynamic_cast<FuncNode*>(func);
  if (f->_type() != BOOL) {
    yyserror("function lambda has incoherent return type");
  }
  int n = f->createDeque().size();
  if (n != 1) {
    yyserror("filter's lambda expects 1 parameters but received %d", n);
  }
}

} // namespace AST
