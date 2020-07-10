#include "y.tab.h"
#include "gen_dot.h"

extern TreeNode* root;

void print_node(TreeNode* tn){
    char buf[1024];
    int count = 0;
    while (tn->child[count] != 0)
    {
        count++;
    }
    
    sprintf(buf, "Token:%d\nnum:%d\nstr:%s\nsibling:%x\nchild count:%d", tn->token, tn->num, tn->str, tn->sibling, count);
    printf("%s", buf);
}

int main(){
    // prepare for failure of parsing

    int ret = yyparse();
    if(ret == 1){
        // error occur during parsing
        printf("ERROR when processing");
        char buf[1024];
        return 1;
    }
    else {
        // everything works fine
        TreeNode* syntax_tree = root;
        // print_node(syntax_tree);
        // generate_dot(syntax_tree, stdout);
        char* buf = gen_dot_str(syntax_tree);
        printf("%s", buf);
    }
    return 0;
}