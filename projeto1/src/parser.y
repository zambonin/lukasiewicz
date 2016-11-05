/*
 * Bison parser file for a language called
 * Łukasiewicz, based on prefix notation.
 *
 * Authors: Douglas Martins, Gustavo Zambonin,
 *          Marcello Klingelfus
 */
%{
  #include <cstring>
  #include "ast.h"
  #include "st.h"

  extern int yylex();
  extern int yylex_destroy();
  extern void yyerror(const char* s, ...);

  /* First symbol table (global scope). */
  ST::SymbolTable* current;

  /* Root of the abstract syntax tree. */
  AST::BlockNode* root;

  /* Temporary variable used to simplify the grammar on declarations. */
  int tmp_t;
%}

/* Bison declaration summary. */

/* Enable more specific syntax errors. */
%define parse.error verbose

/* Instrument the parser for traces. */
%define parse.trace

/* Write a parser header file containing macro definitions for the token
   type names defined in the grammar. */
%defines "include/parser.h"

/* Generate the parser implementation on that file. */
%output "src/parser.cpp"

/* Write an output file containing descriptions of the states and what is
   done for each type of lookahead token in that state. */
%verbose

/* Possible C types for semantic values. */
%union {
  int integer;
  char* decimal;
  bool boolean;
  AST::Node* node;
  AST::BlockNode* block;
  char* name;
}

%destructor { free($$); } <name>
%destructor { free($$); } <decimal>
%destructor { delete $$; } <node>
%destructor { delete $$; } <block>

/* Definition of tokens and their types. */
%token NL COMMA ASSIGN LPAR RPAR LCURLY RCURLY
%token IF THEN ELSE FOR T_INT T_FLOAT T_BOOL FUN RET ARR
%token <integer> INT
%token <decimal> FLOAT
%token <boolean> BOOL
%token <name> ID

/* Nonterminal symbols and their types. */
%type <block> lines else body f-body
%type <node> expr line declaration basic-type iteration decl-array f-decl
%type <integer> is-array d-type ref-cnt
%type <null> program start-scope end-scope

/* Operator precedence. */
%left C_INT C_FLOAT C_BOOL
%left AND OR
%left EQ NEQ GT LT GEQ LEQ
%left PLUS MINUS
%left TIMES DIV
%left UMINUS NOT ADDR REF
%nonassoc error

/* Starting grammar rule. */
%start program

%%

/* Grammar rules. */

/*
 * program
 *
 * Sets the root of the abstract syntax tree and initializes the global scope.
 */
program
  : %empty                        {}
  | start-scope lines end-scope   { root = $2; }
  ;

/*
 * start-scope
 *
 * Initializes a new scope.
 */
start-scope
  : %empty  { current = new ST::SymbolTable(current); }
  ;

/*
 * end-scope
 *
 * Cleans up the current scope and configures the grammar to use its parent.
 */
end-scope
  : %empty
    {
      if (current->external != nullptr) {
        ST::SymbolTable* pt = current;
        current = current->external;
        delete pt;
      }
    }

/*
 * lines
 *
 * Stores every derived line on the abstract syntax tree.
 */
lines
  : line            { $$ = new AST::BlockNode();
                      if ($1 != nullptr) $$->nodeList.push_back($1); }
  | lines line      { if ($2 != nullptr) $1->nodeList.push_back($2); $$ = $1; }
  ;

/*
 * line
 *
 * Defines the structure of valid inputs to the grammar. A valid line can be:
 *   - an empty line;
 *   - individual or multiple assignments and variable declarations. Simple
 *     variables and arrays may not be declared together;
 *   - a conditional branch operation, called `if`;
 *   - a loop operation, called `for`.
 *   - functions, that may be declared using the keyword `fun`.
 */
line
  : NL                  { $$ = 0; tmp_t = 0; }
  | d-type declaration  { $$ = new AST::MessageNode($2); }
  | d-type decl-array   { $$ = new AST::MessageNode($2); }
  | ref-cnt ID ASSIGN expr
    { AST::Node* n = current->useVariable($2);
      if ($1) n = new AST::UnaryOpNode(AST::ref, n);
      $$ = new AST::BinaryOpNode(AST::assign, n, $4); }
  | ref-cnt ID LPAR expr RPAR ASSIGN expr
    { AST::Node* n = current->useVariable($2);
      if ($1) n = new AST::UnaryOpNode(AST::ref, n);
      AST::Node* left = new AST::BinaryOpNode(AST::index, n, $4);
      $$ = new AST::BinaryOpNode(AST::assign, left, $7); }
  | IF expr NL THEN LCURLY NL body else
    { $$ = new AST::IfNode($2, $7, $8); }
  | FOR iteration COMMA expr COMMA iteration LCURLY NL body
    { $$ = new AST::ForNode($2, $4, $6, $9); }
  | d-type is-array FUN ID start-scope LPAR f-decl RPAR f-body
    { $$ = current->newFunction($4, $7, $1 + $2, $9); }
  | prod-error line     { $$ = $2; }
  ;

/*
 * f-body
 *
 * Represents the lines inside a function.
 */
f-body
  : %empty
    { $$ = nullptr; }
  | LCURLY lines RET expr end-scope NL RCURLY
    { $2->nodeList.push_back(new AST::ReturnNode($4));
      $$ = $2; }
  ;

/*
 * f-decl
 *
 * Defines the possible parameters for a function.
 */
f-decl
  : %empty
    { $$ = new AST::Node(); }
  | d-type ID
    { $$ = current->newVariable($2, nullptr, $1, 0, true); }
  | d-type ID LPAR INT RPAR
    { $$ = current->newVariable($2, nullptr, $1 + 3, $4, true); }
  | f-decl COMMA d-type ID
    { $$ = current->newVariable($4, $1, $3, 0, true); }
  | f-decl COMMA d-type ID LPAR INT RPAR
    { $$ = current->newVariable($4, $1, $3 + 3, $6, true); }
  ;

/*
 * body
 *
 * Represents the lines inside `if` or `for` operations.
 */
body
  : RCURLY                                { $$ = new AST::BlockNode(); }
  | start-scope lines end-scope RCURLY    { $$ = $2; }
  ;

/*
 * else
 *
 * Defines the optional `else` clause of an `if` operation.
 */
else
  : %empty                { $$ = new AST::BlockNode(); }
  | ELSE LCURLY NL body   { $$ = $4; }
  ;

/*
 * iteration
 *
 * Defines the initialization of a variable on a `for` operation.
 */
iteration
  : %empty          { $$ = new AST::Node(); }
  | ID ASSIGN expr  { AST::Node* n = current->useVariable($1);
                      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  ;

/*
 * d-type
 *
 * Defines the primitive types accepted by the grammar.
 */
d-type
  : T_INT ref-cnt   { $$ = 0 + $2; tmp_t += 0; }
  | T_FLOAT ref-cnt { $$ = 1 + $2; tmp_t += 1; }
  | T_BOOL ref-cnt  { $$ = 2 + $2; tmp_t += 2; }
  ;

/*
 * ref-cnt
 *
 * Counts how many pointer references are being made.
 */
ref-cnt
  : %empty      { $$ = 0; }
  | ref-cnt REF { $$ = $1 + 6; tmp_t += 6; }
  ;

/*
 * is-array
 *
 * Checks if the return type of a function is an array.
 */
is-array
  : %empty { $$ = 0; }
  | ARR    { $$ = 3; }
  ;

/*
 * declaration
 *
 * Defines the declaration and possible initialization
 * of one or multiple variables of the same type.
 */
declaration
  : ID
    { $$ = current->newVariable($1, nullptr, tmp_t, 0); }
  | ID ASSIGN basic-type
    { AST::Node* n = current->newVariable($1, nullptr, tmp_t, 0);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | declaration COMMA ID
    { $$ = current->newVariable($3, $1, tmp_t, 0); }
  | declaration COMMA ID ASSIGN basic-type
    { AST::Node* n = current->newVariable($3, $1, tmp_t, 0);
      if (n != $1) $$ = new AST::BinaryOpNode(AST::assign, n, $5); }
  ;

/*
 * decl-array
 *
 * Defines the declaration of one or multiple array variables of the same type.
 */
decl-array
  : ID LPAR INT RPAR
    { $$ = current->newVariable($1, nullptr, tmp_t + 3, $3); }
  | decl-array COMMA ID LPAR INT RPAR
    { $$ = current->newVariable($3, $1, tmp_t + 3, $5); }
  ;

/*
 * basic-type
 *
 * Defines the primitive types accepted by the language.
 */
basic-type
  : INT                     { $$ = new AST::IntNode($1); }
  | FLOAT                   { $$ = new AST::FloatNode($1); }
  | BOOL                    { $$ = new AST::BoolNode($1); }
  ;

/*
 * expr
 *
 * Defines the arithmetic, casting and logic operations of the language.
 */
expr
  : basic-type              { $$ = $1; }
  | ID                      { $$ = current->useVariable($1); }
  | expr PLUS expr          { $$ = new AST::BinaryOpNode(AST::add, $1, $3); }
  | expr MINUS expr         { $$ = new AST::BinaryOpNode(AST::sub, $1, $3); }
  | expr TIMES expr         { $$ = new AST::BinaryOpNode(AST::mul, $1, $3); }
  | expr DIV expr           { $$ = new AST::BinaryOpNode(AST::div, $1, $3); }
  | expr AND expr           { $$ = new AST::BinaryOpNode(AST::_and, $1, $3); }
  | expr OR expr            { $$ = new AST::BinaryOpNode(AST::_or, $1, $3); }
  | expr EQ expr            { $$ = new AST::BinaryOpNode(AST::eq, $1, $3); }
  | expr NEQ expr           { $$ = new AST::BinaryOpNode(AST::neq, $1, $3); }
  | expr GT expr            { $$ = new AST::BinaryOpNode(AST::gt, $1, $3); }
  | expr LT expr            { $$ = new AST::BinaryOpNode(AST::lt, $1, $3); }
  | expr GEQ expr           { $$ = new AST::BinaryOpNode(AST::geq, $1, $3); }
  | expr LEQ expr           { $$ = new AST::BinaryOpNode(AST::leq, $1, $3); }
  | MINUS expr %prec UMINUS { $$ = new AST::UnaryOpNode(AST::uminus, $2); }
  | NOT expr                { $$ = new AST::UnaryOpNode(AST::_not, $2); }
  | ADDR expr               { $$ = new AST::UnaryOpNode(AST::addr, $2); }
  | REF expr                { $$ = new AST::UnaryOpNode(AST::ref, $2); }
  | C_INT expr              { $$ = new AST::UnaryOpNode(AST::cast_int, $2); }
  | C_FLOAT expr            { $$ = new AST::UnaryOpNode(AST::cast_float, $2); }
  | C_BOOL expr             { $$ = new AST::UnaryOpNode(AST::cast_bool, $2); }
  | LPAR expr RPAR          { $$ = $2; }
  | ID LPAR expr RPAR       { AST::Node* n = current->useVariable($1);
                              $$ = new AST::BinaryOpNode(AST::index, n, $3); }
  ;

/*
 * prod-error
 *
 * Syntax error handler.
 */
prod-error
  : error NL            { yyerrok; }
  ;

%%

/* Additional C code. */

int main(int argc, char *argv[]) {

  if (argv[1] != nullptr) {
    yydebug = (strcmp(argv[1], "--debug") == 0);
  }

  yyparse();

  if (root != nullptr) {
    root->print(true);
  }

  delete root;
  delete current;

  yylex_destroy();

  return 0;

}
