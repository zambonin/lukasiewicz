#include "st.h"

using namespace ST;

extern SymbolTable* current;

void SymbolTable::addSymbol(SymbolType type, std::string key, AST::Node* symbol) {
  entryList[type][key] = symbol;
}

bool SymbolTable::symbolExistsHere(SymbolType type, char* key) {
  std::string entry(key);
  return (bool) entryList[type].count(entry);
}

bool SymbolTable::symbolExists(SymbolType type, char* key) {
  if (symbolExistsHere(type, key)) {
    return true;
  } else if (external == nullptr) {
    return false;
  }
  return external->symbolExists(type, key);
}

AST::Node* SymbolTable::getVarFromTable(char* key) {
  if (symbolExistsHere(SymbolType::variable, key)) {
    AST::Node* n = entryList[SymbolType::variable][key];
    return new AST::VariableNode(key, nullptr, n->_type(), 0);
  } else if (external == nullptr) {
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

AST::Node* SymbolTable::useVariable(char* key) {
  if (!symbolExists(SymbolType::variable, key)) {
    yyerror("semantic error: undeclared variable %s", key);
  }
  return getVarFromTable(key);
}

AST::Node* SymbolTable::getFuncFromTable(char* key) {
  if (symbolExistsHere(SymbolType::function, key)) {
    return entryList[SymbolType::function][key];
  } else if (external == nullptr) {
    return new AST::Node();
  }
  return external->getFuncFromTable(key);
}

AST::Node* SymbolTable::newFunction(char* key, AST::Node* params,
  int type, AST::BlockNode* contents) {

  if (symbolExistsHere(function, key)) {
    AST::FuncNode* n = dynamic_cast<AST::FuncNode*>(getFuncFromTable(key));
    if (n->contents != nullptr) {
      yyerror("semantic error: re-definition of function %s", key);
    } else {
      n->contents = contents;
    }
    return new AST::Node();
  }

  AST::Node* n = new AST::FuncNode(key, params, type, contents);
  addSymbol(SymbolType::function, key, n);

  return n;

}
