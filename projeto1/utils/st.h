/*
 *  Symbol table structure for a language called
 *  Łukasiewicz, based on prefix notation.
 *
 *  Authors: Douglas Martins, Gustavo Zambonin,
 *           Marcello Klingelfus
 */
#pragma once

#include <map>
#include "ast.h"

extern void yyerror(const char* s, ...);

namespace ST {

  enum VarType {
    integer, decimal, boolean,
    arr_int, arr_dec, arr_bool,
  };

  enum VarKind { variable };

  class Symbol {
  public:
    VarType type;
    VarKind kind;
    bool init;

    Symbol() {}

    Symbol(VarType type, VarKind kind, bool init):
    type(type), kind(kind), init(init) {}

  };

  class SymbolTable {
  public:
    std::map<std::string, Symbol> entryList;
    SymbolTable* external;

    std::map<VarType, std::string> mapVarStr {
      {integer, "integer"}, {decimal, "float"}, {boolean, "boolean"},
      {arr_int, "a_int"}, {arr_dec, "a_float"}, {arr_bool, "a_bool"},
    };

    std::map<std::string, AST::NodeType> mapStrNode = {
      {"integer", AST::INT}, {"float", AST::FLOAT}, {"boolean", AST::BOOL},
      {"a_int", AST::A_INT}, {"a_float", AST::A_FLOAT}, {"a_bool", AST::A_BOOL},
    };

    SymbolTable(SymbolTable* external):
    external(external) {}

    void addSymbol(std::string key, Symbol symbol);
    void initVariable(std::string key);
    bool isInit(std::string key);
    bool varExistsHere(std::string key);
    bool varExists(std::string key);
    std::string getSymbolType(std::string key);
    AST::Node* newVariable(std::string id, AST::Node* next,
                           VarType type, int size);
    AST::Node* assignVariable(std::string id, AST::Node* next);
    AST::Node* useVariable(std::string id);

  };

}
