#include "gen_dot.h"
#include <string.h>

extern char *tokens[] = {
    "Token_if", "Token_else", "Token_int",
    "Token_void", "Token_while", "Token_return",
    "Token_plus", "Token_minus", "Token_multiply", "Token_divide",
    "Token_less", "Token_lessEqual", "Token_more", "Token_moreEqual",
    "Token_equal", "Token_noEqual", "Token_assign", "Token_semicolon",
    "Token_comma", "Toekn_smallBracket_left", "Token_smallBracket_right",
    "Token_middleBracket_left", "Token_middleBracket_right",
    "Token_largeBracket_left", "Token_largeBracket_right",
    "Token_number", "Token_comment", "Token_identifier", "Token_space", "Token_none",
    "Token_func", "Token_compound", "Token_var", "Token_para", "Token_call", "Token_var_dec"};

static void traverse_dot(TreeNode *t,
                         void (*preProc)(TreeNode *),
                         void (*linkProc)(TreeNode *, TreeNode *, Relation r))
{
  if (t != NULL && t != 0x1)
  {
    preProc(t);
    {
      int i;
      for (i = 0; i < MAX_CHILDREN; i++)
      {
        traverse_dot(t->child[i], preProc, linkProc);
        linkProc(t, t->child[i], CHILD);
      }
    }
    traverse_dot(t->sibling, preProc, linkProc);
    linkProc(t, t->sibling, SIBLING);
  }
}

FILE *f;
int counter;

void genProc_pre(TreeNode *t);
void genProc_post(TreeNode *t);
void genProc_link(TreeNode *t, TreeNode *t1, Relation);

char *gen_dot_str(TreeNode *t)
{
  FILE *f = tmpfile();
  generate_dot(t, f);
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  char *buf = (char *)malloc(fsize + 1);
  fread(buf, 1, fsize, f);
  buf[fsize] = 0;
  fclose(f);
  return buf;
}

void generate_dot(TreeNode *t, FILE *fp)
{
  f = fp;
  char *header = "digraph G{\n";
  fprintf(f, "%s", header);
  counter = 1;

  traverse_dot(t, genProc_pre, genProc_link);
  fprintf(f, "}");
}

char *node2word(TreeNode *t)
{
  return tokens[t->token - tokens_offset];
}

void combine2name(char *dst, char *prefix, int postfix)
{
  sprintf(dst, "%s%d", prefix, postfix);
}

void genProc_pre(TreeNode *t)
{
  t->name = (char *)malloc(256);
  char *name = t->name;
  memset(name, 0, 256);
  combine2name(name, node2word(t), counter);
  counter++;
}

void genProc_link(TreeNode *t, TreeNode *t1, Relation r)
{
  if (t1 != NULL)
  {
    char name[256];
    memset(name, 0, 256);
    switch (r)
    {
    case CHILD:
    {
      if (t1 == 0x1)
      {
        fprintf(f, "\t%s -> %s\n", t->name, "\"[]\"");
      }
      else
      {
        fprintf(f, "\t%s -> %s\n", t->name, t1->name);
      }
    }
    break;
    case SIBLING:
    {
      fprintf(f, "\t{rank=same; %s -> %s[dir=none]}\n", t->name, t1->name);
    }
    break;

    default:
      printf("ERROR in gen link.\n");
      break;
    }
  }
}
