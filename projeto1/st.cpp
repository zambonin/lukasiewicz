#include "st.h"

using namespace ST;

extern SymbolTable symbolTable;

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next) {
  if (varExists(id)) {
    yyerror("semantic error: re-declaration of variable %s\n", id.c_str());
  } else {
    Symbol newEntry(integer, variable, 0, false);
    addSymbol(id, newEntry);
  }

  return new AST::VariableNode(id, next);
}

AST::Node* SymbolTable::assignVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  entryList[id].init = true;
  return new AST::VariableNode(id, NULL);
}

AST::Node* SymbolTable::useVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  if (!entryList[id].init) {
    yyerror("error: variable %s not initialized\n", id.c_str());
  }

  return new AST::VariableNode(id, NULL);
}
