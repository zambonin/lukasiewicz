#include "st.h"

using namespace ST;

extern SymbolTable* current;

void SymbolTable::addSymbol(std::string key, Symbol symbol) {
  entryList[key] = symbol;
}

bool SymbolTable::varExistsHere(std::string key) {
  return (bool) entryList.count(key);
}

bool SymbolTable::varExists(std::string key) {
  if (varExistsHere(key)) {
    return true;
  } else if (external == nullptr) {
    return false;
  }
  return external->varExists(key);
}

std::string SymbolTable::getSymbolType(std::string key) {
  if (varExistsHere(key)) {
    return mapVarStr[entryList[key].type];
  } else if (external == nullptr) {
    return "non";
  }
  return external->getSymbolType(key);
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next,
                                    VarType type, int size) {
  if (varExistsHere(id)) {
    yyerror("semantic error: re-declaration of variable %s", id.c_str());
    // new variable is not added to the symbol table and
    // is skipped by returning `next` or an empty node
    return (next != nullptr) ? next : new AST::Node();
  }

  Symbol newEntry(type, variable);
  addSymbol(id, newEntry);
  return new AST::VariableNode(id, next, mapStrNode[mapVarStr[type]], size);
}

AST::Node* SymbolTable::useVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s", id.c_str());
  }

  return new AST::VariableNode(
    id, nullptr, mapStrNode[current->getSymbolType(id)], 0);
}
