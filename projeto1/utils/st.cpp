#include "st.h"

using namespace ST;

extern SymbolTable* current;

void SymbolTable::addSymbol(std::string key, Symbol symbol) {
  entryList[key] = symbol;
}

void SymbolTable::initVariable(std::string key) {
  if (varExistsHere(key)) {
    entryList[key].init = true;
  } else if (external != NULL) {
    external->initVariable(key);
  }
}

bool SymbolTable::isInit(std::string key) {
  if (varExistsHere(key)) {
    return entryList[key].init;
  } else if (external == NULL) {
    return false;
  }
  return external->isInit(key);
}

bool SymbolTable::varExistsHere(std::string key) {
  return (bool) entryList.count(key);
}

bool SymbolTable::varExists(std::string key) {
  if (varExistsHere(key)) {
    return true;
  } else if (external == NULL) {
    return false;
  }
  return external->varExists(key);
}

std::string SymbolTable::getSymbolType(std::string key) {
  if (varExistsHere(key)) {
    return mapVarStr[entryList[key].type];
  } else if (external == NULL && !varExistsHere(key)) {
    return "non";
  }
  return external->getSymbolType(key);
}

AST::Node* SymbolTable::newVariable(std::string id, AST::Node* next,
                                    VarType type, int size) {
  if (varExistsHere(id)) {
    yyerror("semantic error: re-declaration of variable %s\n", id.c_str());
    return (next != NULL) ? next : new AST::Node();
  } else {
    Symbol newEntry(type, variable, false);
    addSymbol(id, newEntry);
  }

  return new AST::VariableNode(id, next, mapStrNode[mapVarStr[type]], size);
}

AST::Node* SymbolTable::assignVariable(std::string id, AST::Node* next) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  current->initVariable(id);
  std::string s = current->getSymbolType(id);
  AST::NodeType type = s == "non" ? AST::ND : mapStrNode[s];

  return new AST::VariableNode(id, next, type, 0);
}

AST::Node* SymbolTable::useVariable(std::string id) {
  if (!varExists(id)) {
    yyerror("semantic error: undeclared variable %s\n", id.c_str());
  }

  if (!isInit(id)){
    yyerror("error: variable %s not initialized\n", id.c_str());
  }

  std::string s = current->getSymbolType(id);

  return new AST::VariableNode(id, NULL, mapStrNode[s], 0);
}
