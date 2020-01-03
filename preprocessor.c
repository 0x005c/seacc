#include <stdlib.h>
#include <string.h>

#include "seacc.h"

struct Token *preprocess() {
  if(consume("#")) {
    struct Token *directive = consume_ident();
    if(!directive) error_at(token->next->str, "identifier expected");
    if(strncmp(directive->str, "include", 7))
      error_at(directive->str, "directives except include are not implemented");
    if(token->kind != TK_STRING_LITERAL)
      error_at(token->str, "string literal expected");
    char *filename = calloc(1, token->len-1);
    strncpy(filename, token->str+1, token->len-2);

    char *code = read_file(filename);
    struct Token *cur = tokenize(code);
    struct Token *head = cur;

    // TODO: check cur->next
    for(; cur->next->next; cur=cur->next);
    // skip TK_EOF
    cur->next = token->next;

    return head;
  }
  return token;
}
