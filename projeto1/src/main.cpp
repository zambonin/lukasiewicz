#include "st.h"

extern AST::BlockNode* root;
extern int yyparse();
extern int yydebug;

int main() {

  yydebug = 1;
  yyparse();
  if (root != NULL) {
    root->print(true);
  }

  return 0;

}
