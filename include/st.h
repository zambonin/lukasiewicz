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
    explicit SymbolTable(SymbolTable* external):
    external(external) {}

    //! Inserts a symbol on this table.
    /*!
     *  \param type     discerns between variable and function.
     *  \param key      string identifier of the symbol.
     *  \param symbol   Node object.
     */
    void addSymbol(SymbolType type, const std::string& key, AST::Node* symbol);

    //! Checks if an identifier is present on this symbol table.
    /*!
     *  \param type     discerns between variable and function.
     *  \param key      string identifier of the symbol.
     */
    bool symbolExistsHere(SymbolType type, const std::string& key);

    //! Returns a variable node inside of a certain symbol.
    /*!
     *  \param key      identifier of the symbol.
     */
    AST::VariableNode* getVarFromTable(char* key);
    AST::VariableNode* getVarFromTable(const std::string& key);

    //! Creates a new node with informations from the table and tokens
    //! from the grammar.
    /*!
     *  \param key      identifier of the symbol.
     *  \param next     pointer to the next node in the case of multiple
     *                  assignments.
     *  \param type     type of the variable.
     *  \param size     size of the array if applicable.
     *  \param isParam  returns a `ParamNode` if applicable.
     */
    AST::Node* newVariable(char* key, AST::Node* next, int type,
                           int size, bool isParam=false);
    AST::Node* newVariable(const std::string& key, AST::Node* next, int type,
                           int size, bool isParam=false);

    //! Returns a function node inside of a certain symbol.
    /*!
     *  \param key      identifier of the symbol.
     */
    AST::FuncNode* getFuncFromTable(char* key);
    AST::FuncNode* getFuncFromTable(const std::string& key);

    //! Creates a new node representing a function, with informations
    //! from the grammar.
    /*!
     *  \param key      identifier of the symbol.
     *  \param params   head node for a linked list of parameters.
     *  \param type     return type of the function.
     *  \param contents body of the function.
     */
    AST::Node* newFunction(char* key, AST::Node* params, int type,
                           AST::BlockNode* contents);
    AST::Node* newFunction(std::string key, AST::Node* params, int type,
                           AST::BlockNode* contents);
  };

} // namespace ST
