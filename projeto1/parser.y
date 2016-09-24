%{
#include "utils/ast.h"
#include "utils/st.h"

extern int yylex();
extern void yyerror(const char* s, ...);

ST::SymbolTable symbolTable;
AST::BlockNode *root;
%}

%define parse.trace

%union {
  int integer;
  AST::Node *node;
  AST::BlockNode *block;
  const char *name;
}

%token NL COMMA ASSIGN LPAR RPAR
%token <integer> INT
%token <name> ID T_INT

%type <block> lines program
%type <node> expr line declaration d-int

%left PLUS MINUS
%left TIMES DIV
%left UMINUS
%nonassoc error

%start program

%%

program
  : %empty          {}
  | lines           { root = $1; }
  ;

lines
  : line            { $$ = new AST::BlockNode();
                      if ($1 != NULL) $$->nodeList.push_back($1); }
  | lines line      { if ($2 != NULL) $1->nodeList.push_back($2); }
  | lines error NL  { yyerrok; }
  ;

line
  : NL              { $$ = 0; }
  | declaration     { $$ = $1; }
  | ID ASSIGN expr  { AST::Node* n = symbolTable.assignVariable($1, NULL);
                      $$ = new AST::BinaryOpNode(AST::assign, n, $3); }
  ;

declaration
  : T_INT d-int NL  { $$ = new AST::MessageNode($2, "int"); }
  ;

d-int
  : ID
    { $$ = symbolTable.newVariable($1, NULL, ST::integer); }
  | ID ASSIGN expr
    { AST::Node* n = symbolTable.newVariable($1, NULL, ST::integer);
      n = symbolTable.assignVariable($1, NULL);
      $$ = new AST::AssignmentNode(n, $3); }
  | d-int COMMA ID
    { $$ = symbolTable.newVariable($3, $1, ST::integer); }
  | d-int COMMA ID ASSIGN expr
    { AST::Node* n = symbolTable.newVariable($3, $1, ST::integer);
      n = symbolTable.assignVariable($3, $1);
      $$ = new AST::AssignmentNode(n, $5); }
  ;

expr
  : INT                     { $$ = new AST::IntNode($1); }
  | ID                      { $$ = symbolTable.useVariable($1); }
  | expr PLUS expr          { $$ = new AST::BinaryOpNode(AST::add, $1, $3); }
  | expr MINUS expr         { $$ = new AST::BinaryOpNode(AST::sub, $1, $3); }
  | expr TIMES expr         { $$ = new AST::BinaryOpNode(AST::mul, $1, $3); }
  | expr DIV expr           { $$ = new AST::BinaryOpNode(AST::div, $1, $3); }
  | LPAR expr RPAR          { $$ = $2; }
  | MINUS expr %prec UMINUS { $$ = new AST::UnaryOpNode(AST::uminus, $2); }
  ;

%%
