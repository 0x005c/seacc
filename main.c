#include "seacc.h"

int printf(char *fmt, ...);

int main(int argc, char **argv) {
  if(argc != 2) {
    error("引数の数が違います");
    return 1;
  }

  filename = argv[1];
  char *code = read_file(filename);
  user_input = code;
  token = tokenize(code);
  token = preprocess();
  program();

  printf(".data\n");
  struct StringLiteral *lit=slit;
  for(; lit; lit=lit->next) gen_lit(lit);

  printf(".data\n");
  struct Var *v=global;
  for(; v; v=v->next) gen_global(v);

  printf(".text\n");
  struct Node *cur=nodes;
  for(; cur; cur=cur->next) gen(cur);

  return 0;
}

