#include "utils/st.h"

extern AST::BlockNode* root;
extern int yyparse();
extern int yydebug;

int main() {
  yyparse();
  if (root != NULL) {
    root->print(true);
  }

  return 0;
}
