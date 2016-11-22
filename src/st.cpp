#include "st.h"

namespace ST {

extern SymbolTable* current;

void SymbolTable::addSymbol(
  SymbolType type, const std::string& key, AST::Node* symbol) {
  entryList[type][key] = symbol;
}

bool SymbolTable::symbolExistsHere(SymbolType type, const std::string& key) {
  return entryList[type].count(key) == 1;
}

AST::VariableNode* SymbolTable::getVarFromTable(char* key) {
  std::string k(key);
  free(key);
  return getVarFromTable(k);
}

AST::VariableNode* SymbolTable::getVarFromTable(const std::string& key) {
  if (symbolExistsHere(SymbolType::variable, key)) {
    AST::Node* n = entryList[SymbolType::variable][key];
    return new AST::VariableNode(key, nullptr, n->_type(),
      dynamic_cast<AST::VariableNode*>(n)->size);
  }
  if (external == nullptr) {
    yyerror("semantic error: undeclared variable %s", key.c_str());
    return new AST::VariableNode(key, nullptr, -1, 0);
  }
  return external->getVarFromTable(key);
}

AST::Node* SymbolTable::newVariable(
  char* key, AST::Node* next, int type, int size, bool isParam) {
  std::string k(key);
  free(key);
  return newVariable(k, next, type, size, isParam);
}

AST::Node* SymbolTable::newVariable(
  const std::string& key, AST::Node* next, int type, int size, bool isParam) {
  if (symbolExistsHere(SymbolType::variable, key)) {
    yyerror("semantic error: re-declaration of variable %s", key.c_str());
    // new variable is not added to the symbol table and
    // is skipped by returning `next` or the old node
    AST::Node* old = getVarFromTable(key);
    if (next != nullptr) {
      delete old;
      return next;
    }
    return old;
  }

  AST::Node* n;
  if (isParam) {
    n = new AST::ParamNode(key, next, type, size);
  } else {
    n = new AST::DeclarationNode(key, next, type, size);
  }
  addSymbol(SymbolType::variable, key, n);
  return n;
}

AST::FuncNode* SymbolTable::getFuncFromTable(char* key) {
  std::string k(key);
  free(key);
  return getFuncFromTable(k);
}

AST::FuncNode* SymbolTable::getFuncFromTable(const std::string& key) {
  if (symbolExistsHere(SymbolType::function, key)) {
    return dynamic_cast<AST::FuncNode*>(entryList[SymbolType::function][key]);
  }
  if (external == nullptr) {
    yyerror("semantic error: undeclared function %s", key.c_str());
    return new AST::FuncNode(key, nullptr, -1, nullptr);
  }
  return external->getFuncFromTable(key);
}

AST::Node* SymbolTable::newFunction(
  char* key, AST::Node* params, int type, AST::BlockNode* contents) {
  std::string k(key);
  free(key);
  return newFunction(k, params, type, contents);
}

AST::Node* SymbolTable::newFunction(
  std::string key, AST::Node* params, int type, AST::BlockNode* contents) {
  if (symbolExistsHere(SymbolType::function, key)) {
    AST::FuncNode* n = getFuncFromTable(key);
    if (contents != nullptr && n->verifyParams(params)) {
      n->contents = contents;
      delete params;
    } else {
      yyerror("semantic error: re-definition of function %s", key.c_str());
    }
    return nullptr;
  }

  AST::Node* n = new AST::FuncNode(key, params, type, contents);
  // make lambda function callable by the symbol
  key = (key == "lambda") ? "λ" : key;
  addSymbol(SymbolType::function, key, n);
  return n;
}

} // namespace ST
