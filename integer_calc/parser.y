%{
#include <iostream>

extern int yylex();
extern void yyerror(const char* s, ...);
%}

%define parse.trace

/* yylval == %union
 * union informs the different ways we can store data
 */
%union {
    int value;
}

/* token defines our terminal symbols (tokens).
 */
%token <value> INT
%token PLUS MINUS TIMES DIV NL

/* type defines the type of our nonterminal symbols.
 * Types should match the names used in the union.
 * Example: %type<node> expr
 */
%type <value> program lines line expr

/* Operator precedence for mathematical operators
 * The latest it is listed, the highest the precedence
 * left, right, nonassoc
 */
%left PLUS MINUS
%left TIMES DIV

/* Starting rule
 */
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
    NL                { $$ = 0; }
    | expr NL         { std::cout << $1 << std::endl; }
    ;

expr:
    INT               { $$ = $1; }
    | expr PLUS expr  { $$ = $1 + $3; }
    | expr MINUS expr { $$ = $1 - $3; }
    | expr TIMES expr { $$ = $1 * $3; }
    | expr DIV expr   { $$ = $1 / $3; }
    ;

%%
