#include <stdio.h>

typedef enum {
  TK_RESERVED,
  TK_IDENT,
  TK_CHAR_LITERAL,
  TK_STRING_LITERAL,
  TK_INT,
  TK_CHAR,
  TK_STRUCT,
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
typedef struct StructUnion StructUnion;

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
  ND_STRUCT,
  ND_STR,
  ND_EQ,
  ND_NE,
  ND_LT,
  ND_LE,
  ND_LVAR,
  ND_GVAR,
  ND_INIT_ARRAY,
  ND_ASSIGN,
  ND_INIT,
  ND_RETURN,
  ND_IF,
  ND_WHILE,
  ND_FOR,
  ND_BLOCK,
  ND_CALL,
  ND_DEFUN,
  ND_ADDR,
  ND_DEREF,
  ND_DOT,
  ND_ARROW,
  ND_NAME,
} NodeKind;

typedef struct Function Function;

typedef struct Node Node;

typedef struct Var Var;

// TODO: clean up
struct Node {
  NodeKind kind;
  Token *token;
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
typedef enum { STRUCT, /* <- cannot cast */ CHAR, INT, ARY, PTR } TType;

struct Type {
  TType ty;
  Type *ptr_to;
  StructUnion *struct_union;
  int size; // always non-zero
};

struct Var {
  Type *type;
  Var *next;
  char *name;
  long initial;
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

struct StructUnion {
  Var *declarators;

  char *name;
  int len; // length of name
  int size;

  StructUnion *next;
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
int compute_const_expr(Node *exp);
Var *find_member(Node *lhs, Token *tok);

Token *token;
char *filename;
char *user_input;
Node *nodes;
Var *global;
Function *functions;
StringLiteral *slit;
StructUnion *structs;
StructUnion *unions;
