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

AST::Node* SymbolTable::getNodeFromTable(std::string key) {
  if (varExistsHere(key)) {
    AST::Node* n = entryList[key].node;
    return new AST::VariableNode(key, nullptr, n->_type(), 0);
  } else if (external == nullptr) {
    return new AST::VariableNode(key, nullptr, AST::ND, 0);
  }
  return external->getNodeFromTable(key);
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next,
                                    AST::NodeType type, int size) {
  if (varExistsHere(id)) {
    yyerror("semantic error: re-declaration of variable %s", id.c_str());
    // new variable is not added to the symbol table and
    // is skipped by returning `next` or an empty node
    return (next != nullptr) ? next : new AST::Node();
  }

  AST::Node* n = new AST::VariableNode(id, next, type, size);
  Symbol newEntry(type, n, variable);
  addSymbol(id, newEntry);

  return n;
}

AST::Node* SymbolTable::useVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s", id.c_str());
  }

  return getNodeFromTable(id);
}
