#include "st.h"

using namespace ST;

extern SymbolTable* current;

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next,
                                    VarType type) {
  if (varExistsHere(id)) {
    yyerror("semantic error: re-declaration of variable %s\n", id.c_str());
    if (next == NULL) {
      return new AST::Node();
    }
    return next;
  } else {
    Symbol newEntry(type, variable, false);
    addSymbol(id, newEntry);
  }
  return new AST::VariableNode(id, next, mapTypes[type]);
}

AST::Node* SymbolTable::assignVariable(std::string id, AST::Node* next) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  current->initVariable(id);
  std::string s = current->getSymbolType(id);
  AST::Node* node = new AST::Node();
  if(s == "non"){
    return new AST::VariableNode(id, next);
  } else {
    return new AST::VariableNode(id, next, node->nodeTypeString[s]);
  }
}

AST::Node* SymbolTable::useVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  if (!isInit(id)){
    yyerror("error: variable %s not initialized\n", id.c_str());
  }
  std::string s = current->getSymbolType(id);
  AST::Node* node = new AST::Node();

  return new AST::VariableNode(id, NULL, node->nodeTypeString[s]);
}
