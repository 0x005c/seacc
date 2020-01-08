#include "seacc.h"

int strncmp(char *l, char *r, long len);
void *calloc(long nmemb, long size);
char *strncpy(char *dest, char *src, long len);

struct Token *preprocess() {
  if(consume("#")) {
    struct Token *directive = consume_ident();
    if(directive == 0) error_at(token->next->str, "identifier expected");
    if(strncmp(directive->str, "include", 7))
      error_at(directive->str, "directives except include are not implemented");
    if(token->kind != TK_STRING_LITERAL)
      error_at(token->str, "string literal expected");
    char *fname = calloc(1, token->len-1);
    strncpy(fname, token->str+1, token->len-2);

    char *code = read_file(fname);
    struct Token *cur = tokenize(code);
    struct Token *head = cur;

    // TODO: check cur->next
    for(; cur->next->next; cur=cur->next) {
    }
    // skip TK_EOF
    cur->next = token->next;

    return head;
  }
  return token;
}
