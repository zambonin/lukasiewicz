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

  //! Kind of symbol stored in the symbol table,
  //! such as variables and functions.
  enum VarKind { variable };

  //! Class that represents a generic symbol inside the table.
  class Symbol {
  public:
    //! Type of variable or function.
    AST::NodeType type;

    //! Original node reference.
    AST::Node* node;

    //! Kind of symbol or function.
    VarKind kind;

    //! Empty constructor left to be used by `map`.
    Symbol() {}

    //! Basic constructor.
    Symbol(AST::NodeType type, AST::Node* node, VarKind kind):
    type(type), node(node), kind(kind) {}

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

    //! Basic constructor.
    SymbolTable(SymbolTable* external):
    external(external) {}

    //! Inserts a symbol on this table.
    /*!
     *  \param key      string identifier of the symbol.
     *  \param symbol   Symbol object.
     */
    void addSymbol(std::string key, Symbol symbol);

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

    //! Returns a node inside of a certain symbol.
    /*!
     *  \param key  string identifier of the symbol.
     */
    AST::Node* getNodeFromTable(std::string key);

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
                           AST::NodeType type, int size);

    //! Used to connect the nodes when multiple ones are declared.
    /*!
     *  \param id     string identifier of the symbol.
     */
    AST::Node* useVariable(std::string id);

  };

}
