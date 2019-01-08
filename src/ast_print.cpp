#include "ast.h"

namespace AST {

/* String representation for the operations. */
static const std::string _bin[] = {
    "+",       "-",        "*",       "/",       "=",      "[index]",
    " [addr]", " [ref]",   "==",      "!=",      ">",      "<",
    ">=",      "<=",       "&",       "|",       " -u",    " !",
    " [int]",  " [float]", " [bool]", " [word]", " [len]", "[append]"};

//! Saves the current indentation status.
static int spaces;

//! Takes a single line of code and indents it with two spaces.
#define _tab(X)                                                                \
  spaces += 2;                                                                 \
  (X);                                                                         \
  spaces -= 2

//! Variadic macro that prevents indentation for any number of lines.
#define _notab(...)                                                            \
  int tmp = spaces;                                                            \
  spaces = 0;                                                                  \
  (__VA_ARGS__);                                                               \
  spaces = tmp;

void IntNode::printInfix() { text(value, 1); }

void FloatNode::printInfix() { text(value, 1); }

void BoolNode::printInfix() { text(value ? "true" : "false", 1); }

void CharNode::printInfix() { text(value, 1); }

void BinaryOpNode::printPrefix() {
  bool space = ((binOp != assign) && (binOp != append));
  text("", static_cast<int>(space));
  text(_bin[binOp], spaces);
  _notab(left->printPrefix(), right->printPrefix());
}

void BinaryOpNode::printInfix() {
  bool space = ((binOp == assign) || (binOp == append));
  _notab(left->printInfix(), text("", static_cast<int>(space)),
         text(_bin[binOp], spaces), right->printInfix());
}

void UnaryOpNode::printInfix() { this->printPrefix(); }

void UnaryOpNode::printPrefix() {
  text(_bin[op], 0);
  node->printPrefix();
}

void VariableNode::printInfix() { text(id, 1); }

void BlockNode::printPrefix() {
  for (Node *n : nodeList) {
    if (n != nullptr) {
      n->printPrefix();
      if (dynamic_cast<FuncNode *>(n) == nullptr && n->_type() != ND) {
        text("\n", 0);
      }
    }
  }
}

void MessageNode::printPrefix() {
  std::string s = (notArray(this)) ? " var:" : ":";
  text(this->_vtype(true) + s, spaces);
  next->printInfix();
}

void IfNode::printPrefix() {
  text("if:", spaces);
  _notab(condition->printPrefix());
  text("\n", 0);
  text("then:\n", spaces);
  _tab(_then->printPrefix());
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    _tab(_else->printPrefix());
  }
}

void ForNode::printPrefix() {
  text("for: ", spaces);
  _notab(assign->printPrefix(), text(",", 0), test->printPrefix(),
         text(", ", 0), iteration->printPrefix());
  text("\n", 0);
  text("do:\n", spaces);
  _tab(body->printPrefix());
}

void FuncNode::printPrefix() {
  if (this->contents != nullptr) {
    text(this->_vtype(true) + " fun: " + this->id + " (params: ", spaces);
    if (params != nullptr) {
      params->printInfix();
    }
    text(")\n", 0);
    _tab(contents->printPrefix());
  } else {
    yyserror("function %s is declared but never defined", this->id.c_str());
  }
}

void ParamNode::printInfix() {
  if (this->_type() != ND) {
    if (next != nullptr) {
      next->printInfix();
      text(", ", 0);
    }
    text(this->_vtype(true) + " " + id, 0);
  }
}

void ReturnNode::printPrefix() {
  text("ret", spaces);
  _notab(next->printPrefix());
}

void FuncCallNode::printPrefix() {
  std::string psize = std::to_string(params->nodeList.size());
  text(" " + function->id + "[" + psize + " params]", spaces);
  for (Node *n : params->nodeList) {
    n->printPrefix();
  }
}

void DeclarationNode::printInfix() {
  if (next != nullptr) {
    next->printInfix();
    text(",", 0);
  }
  std::string s;
  if (!notArray(this)) {
    s = " (size: " + std::to_string(this->size) + ")";
  }
  text(id + s, 1);
}

} // namespace AST
