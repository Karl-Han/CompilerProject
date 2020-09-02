#define ANALYZE FALSE
/* 
 * MIT License
 * 
 * Copyright (c) 2020 Karl Han
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/* 
 * Author: Karl Han
 * Version: 0.0.1
 * Modified date: 2020-07-10
 */

/* 
 * File name: test.c
 * Main usage: the source file of the executable parser. Use stdin as the parameter to 
 *              process syntax analysis. Based on Bison and Flex.
 * 
 */

extern "C"{
    #include "y.tab.h"
    #include "gen_dot.h"
    extern FILE* yyin;
    int yyparse();
    char* gen_dot_str(TreeNode* t);
}
#include "analyze.h"
#include "cgen.h"
#include <cstdio>
// #include <string>
// #include <iostream>
// #include <fstream>
// #include <streambuf>


extern TreeNode *root;
FILE* code = stdout;
bool TraceCode = true;

void print_node(TreeNode *tn)
{
    char buf[1024];
    int count = 0;
    while (tn->child[count] != 0)
    {
        count++;
    }

    sprintf(buf, "Token:%d\nnum:%d\nstr:%s\nsibling:%x\nchild count:%d", tn->token, tn->num, tn->str, tn->sibling, count);
    printf("%s", buf);
}

int main(int argc, char* argv[])
{
    code = stdout;
    if (argc > 1)
    {
        // get file name
        FILE* f = fopen("combine.cpp", "w+");
        FILE* src = fopen(argv[1], "r");
        FILE* ios_file = fopen("ios_wrapper.cpp", "r");
        // std::ifstream src_if(argv[1]);
        // std::ifstream ios_if("ios_wrapper.cpp");
        // string src((std::istreambuf_iterator<char>(src_if)), (std::istreambuf_iterator<char>()));
        // string ios((std::istreambuf_iterator<char>(ios_if)), (std::istreambuf_iterator<char>()));
        // string combine = ios + src;
        // fprintf(f, "%s", combine.c_str());
        char buf[2048];
        memset(buf, 0, 2048);

        while (fgets(buf, sizeof buf, ios_file) != NULL)
            fprintf(f, "%s\n", buf);
        memset(buf, 0, 2048);
        while (fgets(buf, sizeof buf, src) != NULL)
            fprintf(f, "%s\n", buf);

        rewind(f);

        yyin = f;
        fclose(src);
        fclose(ios_file);
    }
    else{
        yyin = stdin;
    }


    // prepare for failure of parsing
    int ret = yyparse();
    TreeNode* syntax_tree;
    if (ret == 1)
    {
        // error occur during parsing
        printf("ERROR when processing");
        char buf[1024];
        return 1;
    }

    // everything works fine
    syntax_tree = root;
    // print_node(syntax_tree);
    // generate_dot(syntax_tree, stdout);
    FILE* dot_fp = fopen("temp.dot", "w");
    char *buf = gen_dot_str(syntax_tree);
    // printf("%s", buf);
    fprintf(dot_fp, buf);

    // buildSymtabs_c(syntax_tree);
    build_symtabs(syntax_tree);
    FILE* fp_symtab = fopen("gcd.symtab", "w");
    print_symtabs(fp_symtab);
    FILE* fp_functab = fopen("gcd.functab", "w");
    print_functabs(fp_functab);
    printf("* Passed build symbol tables.\n");

    type_check(syntax_tree);
    printf("* Passed type checking.\n");
    
    tag_kind(syntax_tree);

    code_generate(syntax_tree);
    return 0;
}
