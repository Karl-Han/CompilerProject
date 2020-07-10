#include "gen_dot.h"

char *st_s[] = {
    "IfK",
    "RepeatK",
    "AssignK",
    "ReadK",
    "WriteK",
    "WhileK",
    "DowhileK",
    "ForK"
};

char *exp_s[] = {
    "OpK",
    "ConstK",
    "IdK",
};


static void traverse_dot(TreeNode *t,
                         void (*preProc)(TreeNode *),
                         void (*linkProc)(TreeNode *, TreeNode *, Relation r))
{
  if (t != NULL)
  {
    preProc(t);
    {
      int i;
      for (i = 0; i < MAXCHILDREN; i++)
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

void generate_dot(TreeNode *t, FILE* fp)
{
  f = fp;
  char* header = "digraph G{\n";
  fprintf(f, "%s", header);
  counter = 1;

  traverse_dot(t, genProc_pre, genProc_link);
  fprintf(f, "}");
}

char *node2word(TreeNode *t)
{
  switch (t->nodekind)
  {
  case ExpK:
  {
    return exp_s[t->kind.exp];
  }
  break;
  case StmtK:
  {
    return st_s[t->kind.stmt];
  }
  break;

  default:
    printf("ERROR in node2word");
    exit(1);
    break;
  }
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
      fprintf(f, "\t%s -> %s\n", t->name, t1->name);
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