#include <iostream>
#include "ast.h"
#include "st.h"

extern AST::BlockNode* root;
extern int yyparse();
extern int yydebug;

int main() {
    yyparse();

    if (root != NULL) {
        root->printTree();
    }

    return 0;
}
