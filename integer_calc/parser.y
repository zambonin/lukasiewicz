%{
#include <iostream>
#include <math.h>

extern int yylex();
extern void yyerror(const char* s, ...);
%}

%define parse.trace

%union {
    int value;
}

%token <value> INT
%token PLUS MINUS TIMES DIV EXP LPAR RPAR NL

%type <value> program lines line expr

%left PLUS MINUS
%left TIMES DIV
%left EXP
%left UMINUS

%start program

%%

program:
    lines
    ;

lines:
    line
    | lines line
    ;

line:
    NL                          { $$ = 0; }
    | expr NL                   { std::cout << "= " << $1 << std::endl; }
    ;

expr:
    INT                         { $$ = $1; }
    | expr PLUS expr            { $$ = $1 + $3; }
    | expr MINUS expr           { $$ = $1 - $3; }
    | expr TIMES expr           { $$ = $1 * $3; }
    | expr DIV expr             { $$ = $1 / $3; }
    | expr EXP expr             { $$ = pow($1, $3); }
    | LPAR expr RPAR            { $$ = $2; }
    | MINUS expr %prec UMINUS   { $$ = -$2; }
    ;

%%
