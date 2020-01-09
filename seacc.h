enum TokenKind {
  TK_RESERVED,
  TK_IDENT,
  TK_CHAR_LITERAL,
  TK_STRING_LITERAL,
  TK_INT,
  TK_LONG,
  TK_VOID,
  TK_CHAR,
  TK_STRUCT,
  TK_UNION,
  TK_RETURN,
  TK_IF,
  TK_WHILE,
  TK_FOR,
  TK_ENUM,
  TK_ELSE,
  TK_SIZEOF,
  TK_NUM,
  TK_EOF,
};

struct Token {
  enum TokenKind kind;
  struct Token *next;
  int val;
  struct StringLiteral *lit;
  char *str;
  char *pos;
  int len;
};

enum NodeKind {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_NUM,
  ND_STRUCT,
  ND_UNION,
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
  ND_NOT,
};

// TODO: clean up
struct Node {
  enum NodeKind kind;
  struct Token *token;
  struct Node *next;

  struct Node *lhs;
  struct Node *rhs;
  struct Node *cond;
  struct Node *body;
  struct Node *elsebody;

  struct Var *var;
  struct Function *func;

  int offset;
  int val;
  int id;
};

// order is important: casted from left to right
enum TType { STRUCT, UNION, /* <- cannot cast */ CHAR, INT, LONG, ARY, PTR };

struct Type {
  enum TType ty;
  struct Type *ptr_to;
  struct StructUnion *struct_union;
  int size; // always non-zero
};

struct Var {
  struct Type *type;
  struct Var *next;
  char *name;
  struct Node *initial; // initial value
  int len;
  int offset;
};

struct Enum {
  char *name;
  int value;
  struct Enum *next;
};

struct Function {
  struct Type *type;
  struct Function *next;
  struct Node *body;
  struct Node *args;
  struct Scope *locals;
  struct Var *params;
  char *name;
  int len;
  int offset;
};

struct StringLiteral {
  char *str;
  int id;
  struct StringLiteral *next;
};

struct StructUnion {
  struct Var *declarators;

  char *name;
  int len; // length of name
  int size;

  struct StructUnion *next;
};

struct Scope {
  struct Scope *parent;
  struct Var *variables;
};

void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

char *read_file(char *path);

struct Token *tokenize(char *p);

struct Node *expr();
void program();
struct Token *preprocess();

struct Type *calc_type(struct Node *node);
int size_of(struct Type *type);
void gen_lit(struct StringLiteral *lit);
void gen_global(struct Var *var);
void gen(struct Node *node);
int compute_const_expr(struct Node *exp);
struct Var *find_member(struct StructUnion *struct_union, struct Token *tok);
struct StructUnion *find_struct(char *name);
struct StructUnion *find_union(char *name);
int consume(char *op);
int consume_kind(enum TokenKind kind);
struct Token *consume_ident();

struct Token *token;
char *filename;
char *user_input;
struct Node *nodes;
struct Scope *globals;
struct Scope *current_scope;
struct Function *functions;
struct StringLiteral *slit;
struct StructUnion *structs;
struct StructUnion *unions;
struct Enum *enums;
