%{
#include "utils/ast.h"
#include "utils/st.h"

extern int yylex();
extern void yyerror(const char* s, ...);


ST::SymbolTable* current;
AST::BlockNode* root;
%}

%define parse.trace

%union {
  int integer;
  char* decimal;
  bool boolean;
  AST::Node* node;
  AST::BlockNode* block;
  const char* name;
}

%token NL COMMA ASSIGN LPAR RPAR LCURLY RCURLY IF THEN ELSE FOR
%token <integer> INT
%token <decimal> FLOAT
%token <boolean> BOOL
%token <name> ID T_INT T_FLOAT T_BOOL

%type <block> lines program else body scoped
%type <node> expr line declaration d-int d-float d-bool
%type <node> decl-assign iteration logical-test

%left C_INT C_FLOAT C_BOOL
%left AND OR
%left EQ NEQ GT LT GEQ LEQ
%left PLUS MINUS
%left TIMES DIV
%left UMINUS NOT
%nonassoc error

%start program

%%
program
  : %empty          {}
  | scoped          { root = $1; }
  | error NL        { yyerrok; std::cout << std::endl; }
  ;

scoped
  :     { current = new ST::SymbolTable(current); }
  lines { if (current->external != NULL)
          current = current->external;
          $$ = $2; }
  ;

lines
  : line            { $$ = new AST::BlockNode();
                      if ($1 != NULL) $$->nodeList.push_back($1); }
  | lines line      { if ($2 != NULL) $1->nodeList.push_back($2); }
  | lines error NL  { yyerrok; std::cout << std::endl; }
  ;

line
  : NL              { $$ = 0; }
  | declaration     { $$ = $1; }
  | ID ASSIGN expr
    { AST::Node* n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | IF expr NL THEN LCURLY NL body else
    { $$ = new AST::IfNode($2, $7, $8); }
  | FOR iteration COMMA logical-test COMMA iteration LCURLY NL body
    { $$ = new AST::ForNode($2, $4, $6, $9); }
  ;

else
  : %empty                { $$ = new AST::BlockNode(); }
  | ELSE LCURLY NL body   { $$ = $4; }
  ;

body
  : scoped RCURLY   { $$ = $1; }
  | RCURLY          { $$ = new AST::BlockNode(); }
  ;

iteration
  : %empty          { $$ = new AST::Node(); }
  | ID ASSIGN expr  { AST::Node* n = current->assignVariable($1, NULL);
                      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  ;

declaration
  : T_INT d-int NL      { $$ = new AST::MessageNode($2); }
  | T_FLOAT d-float NL  { $$ = new AST::MessageNode($2); }
  | T_BOOL d-bool NL    { $$ = new AST::MessageNode($2); }
  ;

d-int
  : ID
    { $$ = current->newVariable($1, NULL, ST::integer); }
  | ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($1, NULL, ST::integer);
      n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | d-int COMMA ID
    { $$ = current->newVariable($3, $1, ST::integer); }
  | d-int COMMA ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($3, $1, ST::integer);
      n = current->assignVariable($3, $1);
      $$ = new AST::BinaryOpNode(AST::assign, n, $5); }
  ;

d-float
  : ID
    { $$ = current->newVariable($1, NULL, ST::decimal); }
  | ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($1, NULL, ST::decimal);
      n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | d-float COMMA ID
    { $$ = current->newVariable($3, $1, ST::decimal); }
  | d-float COMMA ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($3, $1, ST::decimal);
      n = current->assignVariable($3, $1);
      $$ = new AST::BinaryOpNode(AST::assign, n, $5); }
  ;

d-bool
  : ID
    { $$ = current->newVariable($1, NULL, ST::boolean); }
  | ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($1, NULL, ST::boolean);
      n = current->assignVariable($1, NULL);
      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  | d-bool COMMA ID
    { $$ = current->newVariable($3, $1, ST::boolean); }
  | d-bool COMMA ID ASSIGN decl-assign
    { AST::Node* n = current->newVariable($3, $1, ST::boolean);
      n = current->assignVariable($3, $1);
      $$ = new AST::BinaryOpNode(AST::assign, n, $5); }
  ;

decl-assign
  : INT                     { $$ = new AST::IntNode($1); }
  | FLOAT                   { $$ = new AST::FloatNode($1); }
  | BOOL                    { $$ = new AST::BoolNode($1); }
  ;

logical-test
  : expr EQ expr            { $$ = new AST::BinaryOpNode(AST::eq, $1, $3); }
  | expr NEQ expr           { $$ = new AST::BinaryOpNode(AST::neq, $1, $3); }
  | expr GT expr            { $$ = new AST::BinaryOpNode(AST::gt, $1, $3); }
  | expr LT expr            { $$ = new AST::BinaryOpNode(AST::lt, $1, $3); }
  | expr GEQ expr           { $$ = new AST::BinaryOpNode(AST::geq, $1, $3); }
  | expr LEQ expr           { $$ = new AST::BinaryOpNode(AST::leq, $1, $3); }
  ;

expr
  : decl-assign             { $$ = $1; }
  | logical-test            { $$ = $1; }
  | ID                      { $$ = current->useVariable($1); }
  | expr PLUS expr          { $$ = new AST::BinaryOpNode(AST::add, $1, $3); }
  | expr MINUS expr         { $$ = new AST::BinaryOpNode(AST::sub, $1, $3); }
  | expr TIMES expr         { $$ = new AST::BinaryOpNode(AST::mul, $1, $3); }
  | expr DIV expr           { $$ = new AST::BinaryOpNode(AST::div, $1, $3); }
  | expr AND expr           { $$ = new AST::BinaryOpNode(AST::_and, $1, $3); }
  | expr OR expr            { $$ = new AST::BinaryOpNode(AST::_or, $1, $3); }
  | MINUS expr %prec UMINUS { $$ = new AST::UnaryOpNode(AST::uminus, $2); }
  | NOT expr                { $$ = new AST::UnaryOpNode(AST::_not, $2); }
  | C_INT expr              { $$ = new AST::UnaryOpNode(AST::cast_int, $2); }
  | C_FLOAT expr            { $$ = new AST::UnaryOpNode(AST::cast_float, $2); }
  | C_BOOL expr             { $$ = new AST::UnaryOpNode(AST::cast_bool, $2); }
  | LPAR expr RPAR          { $$ = $2; }
  ;

%%
