/*!
 * Symbol table structure for a language called
 * Łukasiewicz, based on prefix notation.
 *
 *  \author Douglas Martins, Gustavo Zambonin, Marcello Klingelfus
 */
#pragma once

#include <map>
#include "ast.h"

extern void yyerror(const char* s, ...);

namespace ST {

  //! Types of variables that can be stored in the symbol table.
  //! May also be used to store return types of functions.
  enum VarType {
    integer, decimal, boolean,
    arr_int, arr_dec, arr_bool,
  };

  //! Kind of symbol stored in the symbol table,
  //! such as variables and functions.
  enum VarKind { variable };

  //! Class that represents a generic symbol inside the table.
  class Symbol {
  public:
    //! Type of variable or function.
    VarType type;

    //! Kind of symbol or function.
    VarKind kind;

    //! Initialization status of the symbol, used to check if a
    //! variable has a value or a function has been defined.
    bool init;

    //! Empty constructor left to be used by `map`.
    Symbol() {}

    //! Basic constructor.
    Symbol(VarType type, VarKind kind, bool init):
    type(type), kind(kind), init(init) {}

  };

  //! Class that represents, essentially, a hashed map between
  //! identifiers and their `Symbol` representation.
  class SymbolTable {
  public:
    //! List of entries within this table.
    std::map<std::string, Symbol> entryList;

    //! Parent table used to represent possible external scopes on the
    //! program, thereby creating a linked structure between the symbol tables.
    SymbolTable* external;

    //! Relationship between types of variables and their string names,
    //! used to print errors.
    std::map<VarType, std::string> mapVarStr {
      {integer, "integer"}, {decimal, "float"}, {boolean, "boolean"},
      {arr_int, "a_int"}, {arr_dec, "a_float"}, {arr_bool, "a_bool"},
    };

    //! Relationship between strings and node types, used to create
    //! nodes from the variable types.
    std::map<std::string, AST::NodeType> mapStrNode = {
      {"integer", AST::INT}, {"float", AST::FLOAT}, {"boolean", AST::BOOL},
      {"a_int", AST::A_INT}, {"a_float", AST::A_FLOAT}, {"a_bool", AST::A_BOOL},
    };

    //! Basic constructor.
    SymbolTable(SymbolTable* external):
    external(external) {}

    //! Inserts a symbol on this table.
    /*!
     *  \param key      string identifier of the symbol.
     *  \param symbol   Symbol object.
     */
    void addSymbol(std::string key, Symbol symbol);

    //! Initializes a variable anywhere on the program, if it exists,
    //! navigating through the scopes.
    /*!
     *  \param key  string identifier of the symbol.
     */
    void initVariable(std::string key);

    //! Checks if an identifier is present on this table.
    /*!
     *  \param key  string identifier of the symbol.
     */
    bool varExistsHere(std::string key);

    //! Checks if an identifier is present anywhere on the program.
    /*!
     *  \param key  string identifier of the symbol.
     */
    bool varExists(std::string key);

    //! Returns a string representing the symbol type.
    /*!
     *  \param key  string identifier of the symbol.
     */
    std::string getSymbolType(std::string key);

    //! Creates a new node with informations from the table and tokens
    //! from the grammar.
    /*!
     *  \param id     string identifier of the symbol.
     *  \param next   pointer to the next node in the case of multiple
     *                assignments.
     *  \param type   type of the variable.
     *  \param size   size of the array if applicable.
     */
    AST::Node* newVariable(std::string id, AST::Node* next,
                           VarType type, int size);

    //! Used to connect the nodes when multiple ones are declared.
    /*!
     *  \param id     string identifier of the symbol.
     *  \param next   pointer to the next node in the case of multiple
     *                assignments.
     */
    AST::Node* assignVariable(std::string id, AST::Node* next);

    //! Returns a new node with information similar to the original.
    /*!
     *  \param id   string identifier of the symbol.
     */
    AST::Node* useVariable(std::string id);

  };

}
