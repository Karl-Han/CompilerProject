#include "y.tab.h"
#include "token.h"

void print_node(TreeNode* tn){
    char buf[1024];
    // str += "Token:" + QString(tn->token);
    // str += "\nnum:" + QString(tn->num);
    // str += "\nstr:" + QString(tn->str);
    sprintf(buf, "Token:%d\nnum:%d\nstr:%s\nsibling:%x", tn->token, tn->num, tn->str, tn->sibling);
    // char buf[32];
    // sprintf(buf, "%x", tn->sibling);
    // str += "\nsibling:" + QString(buf);
    // Dialog* d = new Dialog(nullptr, str);
    // d->show();
    printf("%s", buf);
}

int main(){
    // prepare for failure of parsing
    // std::stringstream buffer;
    // std::streambuf * old = std::cerr.rdbuf(buffer.rdbuf());

    // std::cerr << "This is error message."<<std::endl;

    int ret = yyparse();
    if(ret == 1){
        // error occur during parsing
        // std::string text = buffer.str();
    }
    else {
        // everything works fine
        TreeNode* syntax_tree = (TreeNode*)yylval.tn;
        print_node(syntax_tree);
    }
}