#include <stdio.h>

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_CHAR_LITERAL,
  TK_STRING_LITERAL,
  TK_INT,
  TK_CHAR,
  TK_RETURN,
  TK_IF,
  TK_WHILE,
  TK_FOR,
  TK_ELSE,
  TK_SIZEOF,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;
typedef struct StringLiteral StringLiteral;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  StringLiteral *lit;
  char *str;
  int len;
};

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_STR,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_LVAR,
  ND_GVAR,
  ND_ASSIGN,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_CALL,
  ND_DEFUN,
  ND_ADDR,
  ND_DEREF,
} NodeKind;

typedef struct Function Function;

typedef struct Node Node;

typedef struct Var Var;

// TODO: clean up
struct Node {
  NodeKind kind;
  Node *next;

  Node *lhs;
  Node *rhs;
  Node *cond;
  Node *body;
  Node *elsebody;

  Var *var;
  Function *func;

  int offset;
  int val;
  int id;
};

typedef struct Type Type;

// order is important: casted from left to right
typedef enum { CHAR, INT, ARY, PTR } TType;

struct Type {
  TType ty;
  Type *ptr_to;
  int size; // always non-zero
};

struct Var {
  Type *type;
  Var *next;
  char *name;
  int len;
  int offset;
};

struct Function {
  Type *type;
  Function *next;
  Node *body;
  Node *args;
  Var *locals;
  Var *params;
  char *name;
  int len;
  int offset;
};

struct StringLiteral {
  char *str;
  int id;
  StringLiteral *next;
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

char *read_file(char *path);

Token *tokenize(char *p);

Node *expr();
void program();

Type *calc_type(Node *node);
void gen_lit(StringLiteral *lit);
void gen_global(Var *var);
void gen(Node *node);

Token *token;
char *filename;
char *user_input;
Node *nodes;
Var *global;
Function *functions;
StringLiteral *slit;
