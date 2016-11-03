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

  //! Type of symbol stored in the symbol table,
  //! such as variables and functions.
  enum SymbolType { variable, function };

  //! Class that represents, essentially, a hashed map between
  //! identifiers and their `Symbol` representation.
  class SymbolTable {
  public:
    //! List of entries within this table.
    std::map<SymbolType, std::map<std::string, AST::Node*>> entryList;

    //! Parent table used to represent possible external scopes on the
    //! program, thereby creating a linked structure between the symbol tables.
    SymbolTable* external;

    //! Basic constructor.
    SymbolTable(SymbolTable* external):
    external(external) {}

    //! Inserts a symbol on this table.
    /*!
     *  \param type     discerns between variable and function.
     *  \param key      string identifier of the symbol.
     *  \param symbol   Node object.
     */
    void addSymbol(SymbolType type, std::string key, AST::Node* symbol);

    //! Checks if an identifier is present on this table.
    /*!
     *  \param type     discerns between variable and function.
     *  \param key      string identifier of the symbol.
     */
    bool symbolExistsHere(SymbolType type, char* key);

    //! Checks if an identifier is present anywhere on the program.
    /*!
     *  \param type     discerns between variable and function.
     *  \param key      string identifier of the symbol.
     */
    bool symbolExists(SymbolType type, char* key);

    //! Returns a node inside of a certain symbol.
    /*!
     *  \param key      string identifier of the symbol.
     */
    AST::Node* getVarFromTable(char* key);

    //! Creates a new node with informations from the table and tokens
    //! from the grammar.
    /*!
     *  \param key      string identifier of the symbol.
     *  \param next     pointer to the next node in the case of multiple
     *                  assignments.
     *  \param type     type of the variable.
     *  \param size     size of the array if applicable.
     */
    AST::Node* newVariable(char* key, AST::Node* next, int type, int size);

    //! Used to connect the nodes when multiple ones are declared.
    /*!
     *  \param key      string identifier of the symbol.
     */
    AST::Node* useVariable(char* key);

  };

}
