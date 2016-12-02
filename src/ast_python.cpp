#include "ast.h"

namespace AST {

static const std::string _bin[] = {
  " + ", " - ", " * ", " / ", " = ", "[index]", "", "",
  " == ", " != ", " > ", " < ", " >= ", " <= ", " & ", " | ",
  "-", "(not ", "int(", "float(", "bool(", "str(", "len("
  "", " + ["
};

void IntNode::printPython() {
  text(value, 0);
}

void FloatNode::printPython() {
  text(value, 0);
}

void BoolNode::printPython() {
  text(value ? "True" : "False", 0);
}

void CharNode::printPython() {
  text(value, 0);
}

void BinaryOpNode::printPython() {
  bool specialOp = (binOp == assign || binOp == index || binOp == append);
  if (!specialOp) text("(", 0);
  left->printPython();
  if (binOp != index) {
    text(_bin[binOp], 0),
    right->printPython();
  } else {
    text("[", 0);
    right->printPython();
    text("]", 0);
  }
  if (!specialOp) text(")", 0);
  if (binOp == append) text("]", 0);
}

void UnaryOpNode::printPython() {
  text(_bin[op], 0);
  node->printPython();
  if (op > 16) {
    text(")", 0);
  }
}

void VariableNode::printPython() {
  text(id, 0);
}

void BlockNode::printPython() {
  for (Node* n : nodeList) {
    if (n != nullptr) {
      text("", spaces);
      n->printPython();
      if (n->_type() != ND) {
        text("\n", 0);
      }
    }
  }
}

void MessageNode::printPython() {
  next->printPython();
}

void IfNode::printPython() {
  text("s_context()\n", 0);
  text("if ", spaces);
  _notab(condition->printPython());
  text(":\n", 0);
  _tab(_then->printPython());
  if (!_else->nodeList.empty()) {
    text("else:\n", spaces);
    _tab(_else->printPython());
  }
  text("r_context()\n", spaces);
}

void ForNode::printPython() {
  text("s_context()\n", 0);
  if (assign->_type() != ND) {
    text("", spaces);
    assign->printPython();
    text("\n", 0);
  }
  text("while ", spaces);
  _notab(
    test->printPython(),
    text(":\n", 0));
  _tab(body->printPython());
  if (iteration->_type() != ND) {
    text("", spaces + 2);
    iteration->printPython();
    text("\n", 0);
  }
  text("r_context()\n", spaces);
}

void FuncNode::printPython() {
  text("def " + ((this->id == "lambda") ? "λ" : this->id) + "(", 0);
  if (params != nullptr) {
    params->printPython();
  }
  text("):\n", 0);
  if (this->contents != nullptr) {
    _tab(contents->printPython());
  } else {
    text("pass", spaces);
  }
}

void ParamNode::printPython() {
  if (this->_type() != ND) {
    if (next != nullptr) {
      next->printPython();
      text(", ", 0);
    }
    text(id, 0);
  }
}

void ReturnNode::printPython() {
  text("return ", 0);
  _notab(next->printPython());
}

void FuncCallNode::printPython() {
  text(((function->id == "lambda") ? "λ" : function->id) + "(", 0);
  for (Node* n : params->nodeList) {
    n->printPython();
    if (n != params->nodeList.back()) text(", ", 0);
  }
  text(")", 0);
}

void DeclarationNode::printPython() {
  if (next != nullptr) {
    next->printPython();
    text("\n", 0);
  }
  if (this->init) {
    text(id, 0);
  } else if (!notArray(this)) {
    text(id + " = [0] * " + std::to_string(this->size), 0);
  }
}

} //namespace AST
