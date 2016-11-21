/*
 * Flex scanner file for a language called
 * Łukasiewicz, based on prefix notation.
 *
 * Authors: Douglas Martins, Gustavo Zambonin,
 *          Marcello Klingelfus
 */
%{
  #include <cstdarg>
  #include <readline/readline.h>
  #include <readline/history.h>

  #include "ast.h"
  #include "parser.h"

  extern AST::BlockNode* root;
  extern int yylineno;

  void yyerror(const char* s, ...);
%}

/*
 * Definitions section.
 *
 * `noyywrap` assumes that there are no more files to scan;
 * `nodefault` aborts the program if there is input that
 * does not match any rule;
 * `yylineno` generates a scanner that maintains the number
 *  of the current line being read.
 */

%option noyywrap nodefault yylineno

intgT [0-9]+
deciT ([0-9]*\.[0-9]+)|([0-9]+\.)
boolT "true"|"false"
nameT [a-zA-Z][a-zA-Z0-9_]*
intgC "[int]"
deciC "[float]"
boolC "[bool]"
error [^a-zA-Z0-9_\.\+\-\*\/\,\=\(\)\=\<\>\&\|\!\{\}\[\]\λ \t\n]+

%%

 /* Rules section. */

[ \t]     ;
"+"       { return PLUS; }
"-"       { return MINUS; }
"*"       { return TIMES; }
"/"       { return DIV; }
","       { return COMMA; }
"="       { return ASSIGN; }
"("       { return LPAR; }
")"       { return RPAR; }
"=="      { return EQ; }
"!="      { return NEQ; }
">"       { return GT; }
"<"       { return LT; }
">="      { return GEQ; }
"<="      { return LEQ; }
"&"       { return AND; }
"|"       { return OR; }
"!"       { return NOT; }
"addr"    { return ADDR; }
"{"       { return LCURLY; }
"}"       { return RCURLY; }
"["       { return LBRAC; }
"]"       { return RBRAC; }
\n        { return NL; }
"ref"     { return REF; }
"int"     { return T_INT; }
"float"   { return T_FLOAT; }
"bool"    { return T_BOOL; }
"if"      { return IF; }
"then"    { return THEN; }
"else"    { return ELSE; }
"for"     { return FOR; }
"fun"     { return FUN; }
"ret"     { return RET; }
"array"   { return ARR; }
"map"     { return F_MAP; }
"fold"    { return F_FOLD; }
"filter"  { return F_FILTER; }
"lambda"  { return F_LAMBDA; }
"->"      { return RET_L; }
"λ"       { return L_CALL; }
{intgT}   { yylval.integer = std::atoi(yytext); return INT; }
{deciT}   { yylval.decimal = strdup(yytext); return FLOAT; }
{boolT}   { yylval.boolean = (strcmp(yytext, "true") == 0); return BOOL; }
{intgC}   { return C_INT; }
{deciC}   { return C_FLOAT; }
{boolC}   { return C_BOOL; }
"[len]"   { return LEN; }
{nameT}   { yylval.name = strdup(yytext); return ID; }
{error}   { yyerror("lexical error: unknown symbol %s", yytext); }
.         ;

%%

/* User code section. */

/* Bison standard error output function. */
void yyerror(const char* s, ...) {
  va_list ap;
  va_start(ap, s);
  std::fprintf(stderr, "[Line %d] ", yylineno);
  std::vfprintf(stderr, s, ap);
  std::fprintf(stderr, "\n");
}

/* Creates another buffer state and feeds the `s` arg to it. */
AST::BlockNode* string_read(const char* s) {

  YY_BUFFER_STATE old = YY_CURRENT_BUFFER;
  yy_scan_string(s);
  yyparse();

  yy_delete_buffer(YY_CURRENT_BUFFER);
  yy_switch_to_buffer(old);

  return root;

}

/* Uses the readline library to parse stdin. */
char* rl_read() {

  std::string _input;
  static char * line = nullptr;
  bool reading = true;

  while (reading) {
    if (line != nullptr) {
      add_history(line);
      free(line);
    }

    line = readline("");
    reading = (line != nullptr);

    line = (line == nullptr) ? (char*) "" : line;
    _input += (std::string(line) + "\n");
  }

  _input += "\n";
  return strdup(_input.c_str());

}
