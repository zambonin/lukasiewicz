#include "st.h"

using namespace ST;

extern SymbolTable* current;

void SymbolTable::addSymbol(std::string key, Symbol symbol) {
  entryList[key] = symbol;
}

bool SymbolTable::varExistsHere(char* key) {
  std::string entry(key);
  return (bool) entryList.count(entry);
}

bool SymbolTable::varExists(char* key) {
  if (varExistsHere(key)) {
    return true;
  } else if (external == nullptr) {
    return false;
  }
  return external->varExists(key);
}

AST::Node* SymbolTable::getNodeFromTable(char* key) {
  if (varExistsHere(key)) {
    AST::Node* n = entryList[key].node;
    return new AST::VariableNode(key, nullptr, n->_type(), 0, n->ptr_cnt);
  } else if (external == nullptr) {
    return new AST::VariableNode(key, nullptr, AST::ND, 0, 0);
  }
  return external->getNodeFromTable(key);
}

AST::Node* SymbolTable::newVariable(char* id, AST::Node* next,
                                    AST::NodeType type, int size, int ref) {
  if (varExistsHere(id)) {
    yyerror("semantic error: re-declaration of variable %s", id);
    // new variable is not added to the symbol table and
    // is skipped by returning `next` or the old node

    AST::Node* old = getNodeFromTable(id);
    if (next != nullptr) {
      delete old;
      return next;
    }
    return old;

  }

  AST::Node* n = new AST::VariableNode(id, next, type, size, ref);
  Symbol newEntry(type, n, variable);
  addSymbol(id, newEntry);

  return n;
}

AST::Node* SymbolTable::useVariable(char* id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s", id);
  }

  return getNodeFromTable(id);
}
