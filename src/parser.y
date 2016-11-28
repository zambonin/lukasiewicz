/*
 * Bison parser file for a language called
 * Łukasiewicz, based on prefix notation.
 *
 * Authors: Douglas Martins, Gustavo Zambonin,
 *          Marcello Klingelfus
 */
%{
  #include <cstring>
  #include <unistd.h>
  #include "ast.h"
  #include "st.h"

  extern int yylex();
  extern int yylex_destroy();
  extern void yyerror(const char* s, ...);

  extern AST::BlockNode* string_read(const char*);
  extern char* rl_read();

  /* First symbol table (global scope). */
  ST::SymbolTable* current;

  /* Root of the abstract syntax tree. */
  AST::BlockNode* root;

  /* Temporary variable used to simplify the grammar on declarations. */
  int tmp_t;

  /* Temporary variable used to insert the functor node inside the program. */
  AST::Node* tmp_f;
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
  bool boolean;
  char* word;
  AST::Node* node;
  AST::BlockNode* block;
}

/* Delete symbols automatically discarded. */
%destructor { free($$); } <word>
%destructor { delete $$; } <node>
%destructor { delete $$; } <block>

/* Definition of tokens and their types. */
%token NL COMMA ASSIGN APPEND LPAR RPAR LCURLY RCURLY LBRAC RBRAC
%token IF THEN ELSE FOR T_INT T_FLOAT T_BOOL T_CHAR FUN RET ARR RET_L
%token <integer> INT
%token <boolean> BOOL
%token <word> ID FLOAT CHAR STR F_MAP F_FOLD F_FILTER F_LAMBDA L_CALL

/* Nonterminal symbols and their types. */
%type <block> lines else body f-body f-expr
%type <node> expr v-expr line declaration basic-type iteration f-lambda
%type <node> decl-array decl-lambda decl-func
%type <integer> is-array d-type ref-cnt
%type <word> f-type
%type <null> program start-scope end-scope

/* Operator precedence. */
%left C_INT C_FLOAT C_BOOL C_STR LEN
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

/* Sets the root of the syntax tree and initializes the global scope. */
program
  : %empty                        {}
  | start-scope lines end-scope   { root = $2; }
  ;

/* Initializes a new scope. */
start-scope
  : %empty  { current = new ST::SymbolTable(current); }
  ;

/* Cleans up the current scope and configures the grammar to use its parent. */
end-scope
  : %empty
    {
      if (current->external != nullptr) {
        ST::SymbolTable* pt = current;
        current = current->external;
        delete pt;
      } else {
        delete current;
      }
    }

/* Stores every derived line on the abstract syntax tree. */
lines
  : line
    { $$ = new AST::BlockNode($1); }
  | lines line
    { $1->nodeList.push_back(tmp_f);
      $1->nodeList.push_back($2);
      $$ = $1; }
  ;

/*
 * Defines the structure of valid inputs to the grammar. A valid line can be:
 *   - an empty line;
 *   - individual or multiple assignments and variable declarations. Simple
 *     variables and arrays may not be declared together;
 *   - a conditional branch operation, called `if`;
 *   - a loop operation, called `for`.
 *   - functions, that may be declared using the keywords `fun` or `lambda`.
 */
line
  : NL
    { $$ = 0; tmp_t = 0; tmp_f = 0; }
  | d-type declaration
    { $$ = new AST::MessageNode($2, $1); }
  | d-type decl-array
    { $$ = new AST::MessageNode($2, $1 + 4); }
  | ref-cnt ID ASSIGN expr
    { AST::Node* n = current->getVarFromTable($2);
      if ($1) n = new AST::UnaryOpNode(AST::ref, n);
      $$ = new AST::BinaryOpNode(AST::assign, n, $4); }
  | ref-cnt ID APPEND expr
    { AST::Node* n = current->getVarFromTable($2);
      if ($1) n = new AST::UnaryOpNode(AST::ref, n);
      $$ = new AST::BinaryOpNode(AST::append, n, $4);}
  | ref-cnt ID LBRAC expr RBRAC ASSIGN expr
    { AST::Node* n = current->getVarFromTable($2);
      if ($1) n = new AST::UnaryOpNode(AST::ref, n);
      AST::Node* left = new AST::BinaryOpNode(AST::index, n, $4);
      $$ = new AST::BinaryOpNode(AST::assign, left, $7); }
  | IF expr NL THEN LCURLY NL body else
    { $$ = new AST::IfNode($2, $7, $8); }
  | FOR iteration COMMA expr COMMA iteration LCURLY NL body
    { $$ = new AST::ForNode($2, $4, $6, $9); }
  | d-type is-array FUN ID start-scope LPAR decl-func RPAR f-body end-scope
    { $$ = current->newFunction($4, $7, $1 + $2, $9); }
  | f-lambda
    { $$ = $1; }
  | error line
    { $$ = $2; yyerrok; }
  ;

/* Defines the primitive types accepted by the language. */
basic-type
  : INT   { $$ = new AST::IntNode($1); }
  | FLOAT { $$ = new AST::FloatNode($1); free($1); }
  | BOOL  { $$ = new AST::BoolNode($1); }
  | CHAR  { $$ = new AST::CharNode($1); free($1); }
  | STR   { $$ = new AST::CharNode($1); free($1); }
  ;

/* Defines the primitive types accepted by the grammar. */
d-type
  : T_INT ref-cnt   { $$ = 0 + $2; tmp_t += 0; }
  | T_FLOAT ref-cnt { $$ = 1 + $2; tmp_t += 1; }
  | T_BOOL ref-cnt  { $$ = 2 + $2; tmp_t += 2; }
  | T_CHAR ref-cnt  { $$ = 3 + $2; tmp_t += 3; }
  ;

/* Defines the valid higher order function types. */
f-type
  : F_MAP     { $$ = $1; }
  | F_FOLD    { $$ = $1; }
  | F_FILTER  { $$ = $1; }
  ;

/* Checks if the return type of a function is an array. */
is-array
  : %empty { $$ = 0; }
  | ARR    { $$ = 4; }
  ;

/* Counts how many pointer references are being made. */
ref-cnt
  : %empty      { $$ = 0; }
  | ref-cnt REF { $$ = $1 + 8; tmp_t += 8; }
  ;

/* Defines the declaration and possible initialization
   of one or multiple variables of the same type. */
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

/* Defines the declaration of one or multiple arrays of the same type. */
decl-array
  : ID LBRAC INT RBRAC
    { $$ = current->newVariable($1, nullptr, tmp_t + 4, $3); }
  | decl-array COMMA ID LBRAC INT RBRAC
    { $$ = current->newVariable($3, $1, tmp_t + 4, $5); }
  ;

/* Defines the possible parameters for a function. */
decl-func
  : %empty
    { $$ = nullptr; }
  | d-type ID
    { $$ = current->newVariable($2, nullptr, $1, 0, true); }
  | d-type ID LPAR INT RPAR
    { $$ = current->newVariable($2, nullptr, $1 + 4, $4, true); }
  | decl-func COMMA d-type ID
    { $$ = current->newVariable($4, $1, $3, 0, true); }
  | decl-func COMMA d-type ID LPAR INT RPAR
    { $$ = current->newVariable($4, $1, $3 + 4, $6, true); }
  ;

/* Defines the parameter list for an anonymous function. */
decl-lambda
  : d-type ID
    { $$ = current->newVariable($2, nullptr, $1, 0, true); }
  | decl-lambda COMMA ID
    { $$ = current->newVariable($3, $1, $1->_type(), 0, true); }
  ;

/* Represents the lines inside `if` or `for` operations. */
body
  : RCURLY                                { $$ = new AST::BlockNode(); }
  | start-scope lines end-scope RCURLY    { $$ = $2; }
  ;

/* Defines the optional `else` clause of an `if` operation. */
else
  : %empty                { $$ = new AST::BlockNode(); }
  | ELSE LCURLY NL body   { $$ = $4; }
  ;

/* Defines the initialization of a variable on a `for` operation. */
iteration
  : %empty
    { $$ = new AST::Node(); }
  | ID ASSIGN expr
    { AST::Node* n = current->getVarFromTable($1);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  ;

/* Represents the lines inside a function. */
f-body
  : %empty
    { $$ = nullptr; }
  | LCURLY lines RET expr NL RCURLY
    { $2->nodeList.push_back(new AST::ReturnNode($4)); $$ = $2; }
  ;

/* Defines syntax sugar for an anonymous function. */
f-lambda
  : F_LAMBDA start-scope decl-lambda RET_L expr end-scope
    { AST::BlockNode* c = new AST::BlockNode(new AST::ReturnNode($5));
      $$ = current->newFunction($1, $3, $5->_type(), c); }
  | L_CALL LPAR RPAR
    { current->entryList[ST::SymbolType::function].erase($1);
      $$ = 0; free($1); }
  ;

/* Defines the arithmetic, casting and logic operations of the language. */
expr
  : basic-type              { $$ = $1; }
  | v-expr                  { $$ = $1; }
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
  | C_STR expr              { $$ = new AST::UnaryOpNode(AST::cast_word, $2); }
  | LEN expr                { $$ = new AST::UnaryOpNode(AST::len, $2); }
  | LPAR expr RPAR          { $$ = $2; }
  ;

/* Defines the use of variables and functions. */
v-expr
  : ID
    { $$ = current->getVarFromTable($1); }
  | ID LBRAC expr RBRAC
    { AST::Node* n = current->getVarFromTable($1);
      $$ = new AST::BinaryOpNode(AST::index, n, $3); }
  | ID LPAR f-expr RPAR
    { AST::FuncNode* n = current->getFuncFromTable($1);
      $$ = new AST::FuncCallNode(n, $3); }
  | L_CALL LPAR f-expr RPAR
    { AST::FuncNode* n = current->getFuncFromTable($1);
      $$ = new AST::FuncCallNode(n, $3); }
  | f-type LPAR f-lambda COMMA ID RPAR
    { AST::VariableNode* n = current->getVarFromTable($5);
      AST::HiOrdFuncNode* m = AST::HiOrdFuncNode::chooseFunc($1, $3, n);
      $$ = new AST::FuncCallNode(m, new AST::BlockNode(n));
      tmp_f = m; free($1); }
  ;

/* Defines the valid expressions for a parameter list on a function call. */
f-expr
  : %empty            { $$ = new AST::BlockNode(); }
  | expr              { $$ = new AST::BlockNode($1); }
  | f-expr COMMA expr { $1->nodeList.push_back($3); $$ = $1; }
  ;

%%

/* Additional C code. */

int main(int argc, char **argv) {

  int pyflag = 0, rlflag = 0;
  char c;

  while ((c = getopt(argc, argv, "rdp")) != -1)
    switch (c) {
      case 'r':
        rlflag = 1;
        break;
      case 'd':
        yydebug = 1;
        break;
      case 'p':
        pyflag = 1;
        break;
      default:
        return 1;
    }

  // experimental readline mode
  if (rlflag) {
    string_read(rl_read());
  } else {
    yyparse();
  }

  if (root != nullptr) {
    (pyflag) ? root->printPython() : root->printPrefix();
    delete root;
  }

  yylex_destroy();

  return 0;

}
