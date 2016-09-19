%{
#include <string>
#include <cstdarg>
#include "ast.h"
#include "parser.h"

extern int yylineno;
void yyerror(const char* s, ...);
%}

%option noyywrap nodefault yylineno

nameT [a-zA-Z][a-zA-Z0-9_]*
%%

[ \t]   ;
[0-9]+  { yylval.integer = std::atoi(yytext); return INT; }
"+"     { return PLUS; }
"-"     { return MINUS; }
"*"     { return TIMES; }
"/"     { return DIV; }
","     { return COMMA; }
"="     { return ASSIGN; }
"("     { return LPAR; }
")"     { return RPAR; }
\n      { return NL; }
"int"   { return T_INT; }
{nameT} { yylval.name = strdup(yytext); return ID; }
.       { yyerror("lexical error: unknown symbol %c\n", *yytext); }

%%

void yyerror(const char *s, ...) {
  va_list ap;
  va_start(ap, s);
  std::fprintf(stderr, "[Line %d] ", yylineno);
  std::vfprintf(stderr, s, ap);
}
