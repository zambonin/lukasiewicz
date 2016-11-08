#include "st.h"

namespace ST {

extern SymbolTable* current;

void SymbolTable::addSymbol(
  SymbolType type, const std::string& key, AST::Node* symbol) {
  entryList[type][key] = symbol;
}

bool SymbolTable::symbolExistsHere(SymbolType type, char* key) {
  std::string entry(key);
  return entryList[type].count(entry) == 1;
}

AST::Node* SymbolTable::getVarFromTable(char* key) {
  if (symbolExistsHere(SymbolType::variable, key)) {
    AST::Node* n = entryList[SymbolType::variable][key];
    return new AST::VariableNode(key, nullptr, n->_type(), 0);
  }
  if (external == nullptr) {
    yyerror("semantic error: undeclared variable %s", key);
    return new AST::VariableNode(key, nullptr, -1, 0);
  }
  return external->getVarFromTable(key);
}

AST::Node* SymbolTable::newVariable(
  char* key, AST::Node* next, int type, int size, bool isParam) {
  if (symbolExistsHere(SymbolType::variable, key)) {
    yyerror("semantic error: re-declaration of variable %s", key);
    // new variable is not added to the symbol table and
    // is skipped by returning `next` or the old node
    AST::Node* old = getVarFromTable(key);
    if (next != nullptr) {
      delete old;
      return next;
    }
    return old;
  }

  AST::Node* n = isParam ? new AST::ParamNode(key, next, type, size) :
    new AST::VariableNode(key, next, type, size);
  addSymbol(SymbolType::variable, key, n);
  return n;
}

AST::Node* SymbolTable::getFuncFromTable(char* key) {
  if (symbolExistsHere(SymbolType::function, key)) {
    return entryList[SymbolType::function][key];
  }
  if (external == nullptr) {
    yyerror("semantic error: undeclared function %s", key);
    return new AST::FuncNode(key, nullptr, -1, nullptr);
  }
  return external->getFuncFromTable(key);
}

AST::Node* SymbolTable::newFunction(
  char* key, AST::Node* params, int type, AST::BlockNode* contents) {

  // workaround: removes duplicate variables from parent symbol table
  AST::ParamNode* it = dynamic_cast<AST::ParamNode*>(params);
  while (it != nullptr) {
    entryList[SymbolType::variable].erase(it->id);
    it = dynamic_cast<AST::ParamNode*>(it->next);
  }

  if (symbolExistsHere(SymbolType::function, key)) {
    AST::FuncNode* n = dynamic_cast<AST::FuncNode*>(getFuncFromTable(key));
    if (n->contents != nullptr || n->contents == contents) {
      // needs to check number and names of parameters
      yyerror("semantic error: re-definition of function %s", key);
    } else {
      n->contents = contents;
      delete params;
    }
    return new AST::ParamNode(key, nullptr, -1, 0);
  }

  AST::Node* n = new AST::FuncNode(key, params, type, contents);
  addSymbol(SymbolType::function, key, n);
  return n;
}

} // namespace ST
