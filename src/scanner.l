/*
 * Flex scanner file for a language called
 * Łukasiewicz, based on prefix notation.
 *
 * Authors: Douglas Martins, Gustavo Zambonin,
 *          Marcello Klingelfus
 */
%{
  #include <cstdarg>
  #include "ast.h"
  #include "parser.h"

  extern AST::BlockNode* root;
  extern int yylineno;

  void yyerror(const char* s, ...);
  char* reduce_char(char*);
%}

/*
 * Definitions section.
 *
 * `noyywrap` assumes that there are no more files to scan;
 * `nodefault` aborts the program if there is input that
 * does not match any rule;
 * `yylineno` generates a scanner that maintains the number
 *  of the current line being read;
 * `noinput` prevents `yyinput` from being generated automatically.
 * `nounput` prevents `yyunput` from being generated automatically.
 */

%option noyywrap nodefault yylineno noinput nounput


chars   [ !#-&(-~]
intgT   [0-9]+
deciT   ([0-9]*\.[0-9]+)|([0-9]+\.)
boolT   "true"|"false"
nameT   [a-zA-Z][a-zA-Z0-9_]*
charT   [\']{chars}+[\']
wordT   [\"]{chars}*[\"]
error   [^ -~\λ\t\n]+
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
"char"    { return T_CHAR; }
"if"      { return IF; }
"then"    { return THEN; }
"else"    { return ELSE; }
"for"     { return FOR; }
"fun"     { return FUN; }
"ret"     { return RET; }
"array"   { return ARR; }
"->"      { return RET_L; }
"<-"      { return APPEND; }
"map"     { yylval.word = strdup(yytext); return F_MAP; }
"fold"    { yylval.word = strdup(yytext); return F_FOLD; }
"filter"  { yylval.word = strdup(yytext); return F_FILTER; }
"lambda"  { yylval.word = strdup(yytext); return F_LAMBDA; }
"λ"       { yylval.word = strdup(yytext); return L_CALL; }
{intgT}   { yylval.integer = std::atoi(yytext); return INT; }
{boolT}   { yylval.boolean = (strcmp(yytext, "true") == 0); return BOOL; }
{nameT}   { yylval.word = strdup(yytext); return ID; }
{deciT}   { yylval.word = strdup(yytext); return FLOAT; }
{charT}   { yylval.word = reduce_char(strdup(yytext)); return CHAR; }
{wordT}   { yylval.word = strdup(yytext); return STR; }
"[int]"   { return C_INT; }
"[float]" { return C_FLOAT; }
"[bool]"  { return C_BOOL; }
"[word]"  { return C_STR; }
"[len]"   { return LEN; }
{error}   { yyerror("lexical error: unknown symbol %s", yytext); }
"#".*     { /* single-line comments */ }
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

/* Semantic error function. */
void yyserror(const char* s, ...) {

  va_list ap;
  va_start(ap, s);
  std::fprintf(stderr, "[Line %d] semantic error: ", yylineno);
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

/* Truncates a single-quoted word to its first character. */
char* reduce_char(char* s) {

  if (strlen(s) > 3) {
    char _new[4] = {'\'', s[1], '\'', '\0'};
    char* p = strdup(_new);
    yyerror("warning: multicharacter value truncated to %s", p);
    free(s);
    return p;
  } else {
    return s;
  }

}
