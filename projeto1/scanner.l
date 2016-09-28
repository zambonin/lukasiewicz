%{
#include <string>
#include <cstdarg>
#include "utils/ast.h"
#include "parser.h"

extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

intgT [0-9]+
deciT ([0-9]*\.[0-9]+)|([0-9]+\.)
boolT "true"|"false"
nameT [a-zA-Z][a-zA-Z0-9_]*
error [^a-zA-Z0-9_\.\+\-\*\/\,\=\(\)\=\<\>\&\|\!\[\] \t\n]+
intgC "[int]"
deciC "[float]"
boolC "[bool]"

%%

[ \t]   ;
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return TIMES; }
"/"     { return DIV; }
","     { return COMMA; }
"="     { return ASSIGN; }
"("     { return LPAR; }
")"     { return RPAR; }
"=="    { return EQ; }
"!="    { return NEQ; }
">"     { return GT; }
"<"     { return LT; }
">="    { return GEQ; }
"<="    { return LEQ; }
"&"     { return AND; }
"|"     { return OR; }
"!"     { return NOT; }
\n      { return NL; }
"int"   { return T_INT; }
"float" { return T_FLOAT; }
"bool"  { return T_BOOL; }
{intgT} { yylval.integer = std::atoi(yytext); return INT; }
{deciT} { yylval.decimal = strdup(yytext); return FLOAT; }
{boolT} { yylval.boolean = (strcmp(yytext, "true") == 0); return BOOL; }
{intgC} { return C_INT; }
{deciC} { return C_FLOAT; }
{boolC} { return C_BOOL; }
{nameT} { yylval.name = strdup(yytext); return ID; }
{error} { yyerror("lexical error: unknown symbol %s\n", yytext); }
.       ;

%%

void yyerror(const char* s, ...) {
  va_list ap;
  va_start(ap, s);
  std::fprintf(stderr, "[Line %d] ", yylineno);
  std::vfprintf(stderr, s, ap);
}
