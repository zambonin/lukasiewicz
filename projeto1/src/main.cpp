#include <cstring>
#include "ast.h"

extern AST::BlockNode* root;
extern int yyparse();
extern int yydebug;

int main(int argc, char *argv[]) {

  if (argv[1] != nullptr) {
    yydebug = (strcmp(argv[1], "--debug") == 0);
  }

  yyparse();
  if (root != nullptr) {
    root->print(true);
  }

  return 0;

}
