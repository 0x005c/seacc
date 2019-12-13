#include <stdio.h>

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_RETURN,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_LVAR,
  ND_ASSIGN,
  ND_RETURN,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
  int offset;
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

Token *tokenize(char *p);

Node *expr();
void program();

void gen(Node *node);

Token *token;
char *user_input;
Node *code[100];
LVar *locals;