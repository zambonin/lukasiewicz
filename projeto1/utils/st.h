/*
 *  Symbol table structure for a language called
 *  Łukasiewicz, based on prefix notation.
 *
 *  Authors: Douglas Martins, Gustavo Zambonin,
 *           Marcello Klingelfus
 */
#pragma once

#include "ast.h"

 extern void yyerror(const char* s, ...);

 namespace ST {

  // Enumeration of variable types.
  enum VarType { integer, decimal, boolean };

  // Enumeration of variable kinds.
  enum VarKind { variable };

  class Symbol {
  public:
    // Type of symbol.
    VarType type;

    // Kind of symbol.
    VarKind kind;

    // Status of the symbol initialization if it is a variable.
    bool init;

    // Constructor for the Symbol class.
    Symbol(VarType type, VarKind kind, bool init):
    type(type), kind(kind), init(init) {}

    // Initial status for the Symbol object.
    Symbol() {
      kind = variable;
      init = false;
    }
  };

  class SymbolTable {
  public:
    // Map that holds pairs consisting of a variable
    // name and its related Symbol object.
    std::map<std::string, Symbol> entryList;

    std::map<VarType, AST::NodeType> mapTypes = {
      {integer, AST::INT}, {decimal, AST::FLOAT}, {boolean, AST::BOOL},
    };

    std::map<VarType, std::string> typeString {
      {integer, "integer"}, {decimal, "float"}, {boolean, "boolean"},
    };

    SymbolTable* external;

    // Constructor for the SymbolTable class.
    SymbolTable() {}

    SymbolTable(SymbolTable* external):
    external(external) {}

    // Searches for a given key, representing a variable, on the symbol
    // table. Since all keys are unique, it may only return 0 or 1.
    bool varExists(std::string key) {
      if (varExistsHere(key)) {
        return true;
      } else {
        if (external == NULL) {
          return false;
        } else {
          return external->varExists(key);
        }
      }
    }

    bool varExistsHere(std::string key) {
      return (bool) entryList.count(key);
    }

    // Adds an entry to the symbol table map.
    void addSymbol(std::string key, Symbol symbol) {
      entryList[key] = symbol;
    }

    std::string getSymbolType(std::string key) {
      if(varExistsHere(key)) {
        return typeString[entryList[key].type];
      } else {
          if(external == NULL && !varExistsHere(key)) {
            return "non";
          } else {
            return external->getSymbolType(key);
          }
      }
    }

    void initVariable(std::string key) {
      if (varExistsHere(key)) {
        entryList[key].init = true;
      } else {
        if (external != NULL) {
          external->initVariable(key);
        }
      }
    }

    bool isInit(std::string key) {
      if (varExistsHere(key)) {
        return entryList[key].init;
      } else {
        return external->isInit(key);
      }
    }

    // Creates a new variable inside the map.
    AST::Node* newVariable(std::string id, AST::Node* next, VarType type);

    // Sets a value to some variable.
    AST::Node* assignVariable(std::string id, AST::Node* next);

    // Creates a copy of the variable named `id` for
    // references on the code.
    AST::Node* useVariable(std::string id);
  };

}
