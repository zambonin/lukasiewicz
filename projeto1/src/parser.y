/*
 * Bison parser file for a language called
 * Łukasiewicz, based on prefix notation.
 *
 * Authors: Douglas Martins, Gustavo Zambonin,
 *          Marcello Klingelfus
 */
%{
  #include "ast.h"
  #include "st.h"

  extern int yylex();
  extern void yyerror(const char* s, ...);

  /* First symbol table (global scope). */
  ST::SymbolTable* current;

  /* Root of the abstract syntax tree. */
  AST::BlockNode* root;

  /* Temporary variable used to simplify the grammar on declarations. */
  ST::VarType temp;
%}

%define parse.trace

/* Enable more specific syntax errors. */
%define parse.error verbose

/* Possible C types for semantic values. */
%union {
  int integer;
  char* decimal;
  bool boolean;
  AST::Node* node;
  AST::BlockNode* block;
  const char* name;
}

/* Definition of tokens and their types. */
%token NL COMMA ASSIGN LPAR RPAR LCURLY RCURLY IF THEN ELSE FOR
%token <integer> INT
%token <decimal> FLOAT
%token <boolean> BOOL
%token <name> ID T_INT T_FLOAT T_BOOL

/* Nonterminal symbols and their types. */
%type <block> lines program else body scope
%type <node> expr line declaration d-type basic-type iteration decl-array start-scope

/* Operator precedence. */
%left C_INT C_FLOAT C_BOOL
%left AND OR
%left EQ NEQ GT LT GEQ LEQ
%left PLUS MINUS
%left TIMES DIV
%left UMINUS NOT
%nonassoc error

/* Starting grammar rule. */
%start program

%%

/* Grammar rules. */

/*
 * program
 *
 * Sets the root of the abstract syntax tree
 * to the beginning of the global scope.
 */
program
  : %empty              {}
  | start-scope scope   { root = $2; }
  ;

/*
 * start-scope
 *
 * Initializes a new scope whenever a for or if is derived.
 */
start-scope
  : %empty  { current = new ST::SymbolTable(current); }
  ;

scope
  : lines   { if (current->external != NULL) current = current->external;
              $$ = $1; }
  ;

/*
 * lines
 *
 * Stores every derived line on the abstract syntax tree.
 */
lines
  : line            { $$ = new AST::BlockNode();
                      if ($1 != NULL) $$->nodeList.push_back($1); }
  | lines line      { if ($2 != NULL) $1->nodeList.push_back($2); }
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
 */
line
  : NL                      { $$ = 0; }
  | d-type declaration      { $$ = new AST::MessageNode($2, " var:"); }
  | d-type decl-array       { $$ = new AST::MessageNode($2, " array:"); }
  | ID ASSIGN expr
    { AST::Node* n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | ID LPAR expr RPAR ASSIGN expr
    { AST::Node* n = current->assignVariable($1, NULL);
      AST::Node* left = new AST::BinaryOpNode(AST::index, n, $3);
      $$ = new AST::BinaryOpNode(AST::assign, left, $6); }
  | IF expr NL THEN LCURLY NL body else
    { $$ = new AST::IfNode($2, $7, $8); }
  | FOR iteration COMMA expr COMMA iteration LCURLY NL body
    { $$ = new AST::ForNode($2, $4, $6, $9); }
  | error NL line           { yyerrok; $$ = $3; }
  ;

/*
 * body
 *
 * Represents the lines inside `if` or `for` operations.
 */
body
  : RCURLY                      { $$ = new AST::BlockNode(); }
  | start-scope scope RCURLY    { $$ = $2; }
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
 * operation
 *
 * Defines the initialization of a variable on a `for` operation.
 */
iteration
  : %empty          { $$ = new AST::Node(); }
  | ID ASSIGN expr  { AST::Node* n = current->assignVariable($1, NULL);
                      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  ;

/*
 * d-type
 *
 * Defines the primitive types accepted by the grammar on
 * a declaration production.
 */
d-type
  : T_INT   { temp = ST::integer; }
  | T_FLOAT { temp = ST::decimal; }
  | T_BOOL  { temp = ST::boolean; }
  ;

/*
 * declaration
 *
 * Defines the declaration and possible initialization
 * of one or multiple variables of the same type.
 */
declaration
  : ID
    { $$ = current->newVariable($1, NULL, temp, 0); }
  | ID ASSIGN basic-type
    { AST::Node* n = current->newVariable($1, NULL, temp, 0);
      n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | declaration COMMA ID
    { $$ = current->newVariable($3, $1, temp, 0); }
  | declaration COMMA ID ASSIGN basic-type
    { AST::Node* n = current->newVariable($3, $1, temp, 0);
      n = current->assignVariable($3, $1);
      $$ = new AST::BinaryOpNode(AST::assign, n, $5); }
  ;

/*
 * decl-array
 *
 * Defines the declaration of one or multiple array variables of the same type.
 */
decl-array
  : ID LPAR INT RPAR
    { ST::VarType t = static_cast<ST::VarType>(static_cast<int>(temp) + 3);
      $$ = current->newVariable($1, NULL, t, $3); }
  | decl-array COMMA ID LPAR INT RPAR
    { ST::VarType t = static_cast<ST::VarType>(static_cast<int>(temp) + 3);
      $$ = current->newVariable($3, $1, t, $5); }
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
  | C_INT expr              { $$ = new AST::UnaryOpNode(AST::cast_int, $2); }
  | C_FLOAT expr            { $$ = new AST::UnaryOpNode(AST::cast_float, $2); }
  | C_BOOL expr             { $$ = new AST::UnaryOpNode(AST::cast_bool, $2); }
  | LPAR expr RPAR          { $$ = $2; }
  | ID LPAR expr RPAR       { AST::Node* n = current->useVariable($1);
                              $$ = new AST::BinaryOpNode(AST::index, n, $3); }
  ;

%%

/* Additional C code. */
