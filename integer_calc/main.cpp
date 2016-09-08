#include "stdio.h"

extern int yyparse();
void prompt();

int main() {

    prompt();
    yyparse();

    return 0;

}

void prompt() {
    printf("> ");
}
