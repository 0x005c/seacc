#include "seacc.h"

#define NULL 0

Token *token;

char *user_input;

int printf(char *fmt, ...);

int main(int argc, char **argv) {
  if(argc != 2) {
    error("引数の数が違います");
    return 1;
  }

  slit = NULL;
  nodes = NULL;
  global = NULL;
  functions = NULL;
  structs = NULL;
  unions = NULL;

  filename = argv[1];
  char *code = read_file(filename);
  user_input = code;
  token = tokenize(code);
  program();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");

  printf(".data\n");
  for(StringLiteral *lit=slit; lit; lit=lit->next) gen_lit(lit);
  printf(".data\n");
  for(Var *cur=global; cur; cur=cur->next) gen_global(cur);
  printf(".text\n");
  for(Node *cur = nodes; cur; cur = cur->next) gen(cur);

  return 0;
}

