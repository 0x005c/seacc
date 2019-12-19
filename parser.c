#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "seacc.h"

int get_size(Type *type) {
  switch(type->ty) {
    case INT:
    case PTR:
      return 8;
    case ARY:
      return type->size;
    default:
      return -1;
  }
}

bool ptr_like(Node *node) {
  Type *t = calc_type(node);
  if(t->ty == PTR || t->ty == ARY) return true;
  return false;
}

Type anonymous_int = {.ty = INT};

Type *larger_type(Type *a, Type *b) {
  return a->ty > b->ty ? a : b;
}

Type *calc_type(Node *node) {
  Type *l, *r, *type;
  switch(node->kind) {
    case ND_ADD:
    case ND_SUB:
    case ND_MUL:
    case ND_DIV:
      l = calc_type(node->lhs);
      r = calc_type(node->rhs);
      return larger_type(l, r);
    case ND_EQ:
    case ND_NE:
    case ND_LT:
    case ND_LE:
      return &anonymous_int;
    case ND_LVAR:
      return node->lvar->type;
    case ND_ASSIGN:
      return calc_type(node->lhs);
    case ND_CALL:
      // XXX: return type is redarded as int
      return &anonymous_int;
    case ND_ADDR:
      type = calloc(1, sizeof(Type));
      type->ty = PTR;
      type->ptr_to = calc_type(node->lhs);
      return type;
    case ND_NUM:
      return &anonymous_int;
    case ND_DEREF:
      return calc_type(node->lhs)->ptr_to;
    default:
      error("Cannot calculate type on compiliation");
  }
}

int compute_const_expr(Node *exp) {
  switch(exp->kind) {
    case ND_ADD:
      return compute_const_expr(exp->lhs) + compute_const_expr(exp->rhs);
    case ND_SUB:
      return compute_const_expr(exp->lhs) - compute_const_expr(exp->rhs);
    case ND_MUL:
      return compute_const_expr(exp->lhs) * compute_const_expr(exp->rhs);
    case ND_DIV:
      return compute_const_expr(exp->lhs) / compute_const_expr(exp->rhs);
    case ND_NUM:
      return exp->val;
    case ND_LVAR:
    case ND_CALL:
    case ND_DEREF:
      error("Variable length array is not supported");
    case ND_ASSIGN:
      return compute_const_expr(exp->rhs);
    default:
      error("Unexpected node. Cannot compute expression value on compile");
  }
}

bool consume(char *op) {
  if(token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    return false;
  token = token->next;
  return true;
}

bool consume_kind(TokenKind kind) {
  if(token->kind == kind) {
    token = token->next;
    return true;
  }
  return false;
}

bool check_prefix() {
  switch(token->kind) {
    case TK_INT:
      return true;
    default:
      return false;
  }
}

Token *consume_ident() {
  if(token->kind != TK_IDENT) return NULL;

  Token *tok = token;
  token = token->next;
  return tok;
}

void expect(char *op) {
  if(token->kind != TK_RESERVED ||
      strlen(op) != token->len ||
      memcmp(token->str, op, token->len))
    error_at(token->str, "'%s'ではありません", op);
  token = token->next;
}

int expect_number() {
  if(token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

LVar *find_lvar(Token *tok) {
  for(LVar *var = nodes->func->locals; var; var = var->next)
    if(var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

/* primary = "(" expr ")"
 *         | ident "(" (expr ",")* expr? ")"
 *         | ident
 *         | number
 */
Node *primary() {
  if(consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if(tok) {
    if(consume("(")) {
      Function *func = calloc(1, sizeof(Function));
      func->name = tok->str;
      func->len = tok->len;
      if(consume(")")) func->args = NULL;
      else {
        Node head = {.next = NULL};
        Node *cur = &head;
        for(;;) {
          cur->next = expr();
          cur = cur->next;
          if(!consume(",")) {
            expect(")");
            break;
          }
        }
        func->args = head.next;
      }
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->next = NULL;
      node->func = func;
      return node;
    }
    else {
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar(tok);
      if(lvar) {
        node->offset = lvar->offset;
        node->lvar = lvar;
      }
      else {
        error_at(tok->str, "Unexpected token");
      }
      return node;
    }
  }

  return new_node_num(expect_number());
}

/*
 * postfix = primary ("[" expr "]")*
 */
Node *postfix() {
  Node *node = primary();
  for(;;) {
    if(consume("[")) {
      node = new_node(ND_ADD, node, expr());
      node = new_node(ND_DEREF, node, NULL);
      expect("]");
      continue;
    }
    return node;
  }
}

/*
 * unary = "+"? postfix
 *       | "-"? postfix
 *       | "*" unary
 *       | "&" unary
 *       | "sizeof" unary
 *       | postfix
 */
Node *unary() {
  if(consume("+"))
    return postfix();
  if(consume("-"))
    return new_node(ND_SUB, new_node_num(0), postfix());
  if(consume("*"))
    return new_node(ND_DEREF, unary(), NULL);
  if(consume("&"))
    return new_node(ND_ADDR, unary(), NULL);
  if(consume_kind(TK_SIZEOF))
    return new_node_num(get_size(calc_type(unary())));
  return postfix();
}

Node *mul() {
  Node *node = unary();

  for(;;) {
    if(consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if(consume("/"))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  for(;;) {
    if(consume("+")) {
      Node *rhs = mul();
      bool lp = ptr_like(node);
      bool rp = ptr_like(rhs);
      if(lp && !rp)
        rhs = new_node(ND_MUL, rhs, new_node_num(get_size(calc_type(node)->ptr_to)));
      else if(!lp && rp)
        node = new_node(ND_MUL, node, new_node_num(get_size(calc_type(rhs)->ptr_to)));
      else if(lp && rp)
        // NOTE: does this behaviour follow the standard?
        error("[Compile error] cannot add pointer to pointer");
      node = new_node(ND_ADD, node, rhs);
    }
    else if(consume("-")) {
      Node *rhs = mul();
      bool lp = ptr_like(node);
      bool rp = ptr_like(rhs);
      if(lp && !rp)
        rhs = new_node(ND_MUL, rhs, new_node_num(get_size(calc_type(node)->ptr_to)));
      else if(!lp && rp)
        node = new_node(ND_MUL, node, new_node_num(get_size(calc_type(rhs)->ptr_to)));
      else if(lp && rp)
        // NOTE: does this behaviour follow the standard?
        error("[Compile error] cannot add pointer to pointer");
      node = new_node(ND_SUB, node, rhs);
    }
    else return node;
  }
}

Node *relational() {
  Node *node = add();
  for(;;) {
    if(consume("<"))
      node = new_node(ND_LT, node, add());
    else if(consume("<="))
      node = new_node(ND_LE, node, add());
    else if(consume(">"))
      node = new_node(ND_LT, add(), node);
    else if(consume(">="))
      node = new_node(ND_LE, add(), node);
    else return node;
  }
}

Node *equality() {
  Node *node = relational();
  for(;;) {
    if(consume("=="))
      node = new_node(ND_EQ, node, relational());
    else if(consume("!="))
      node = new_node(ND_NE, node, relational());
    else return node;
  }
}

Node *assign() {
  Node *node = equality();
  if(consume("="))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *expr() {
  return assign();
}

/*
 * prefix = "int" "*"*
 */
Type *prefix() {
  if(!consume_kind(TK_INT)) error_at(token->str, "keyword \"int\" expected");
  Type *type = calloc(1, sizeof(Type));
  type->ty = INT;
  type->ptr_to = NULL;
  while(consume("*")) {
    Type *tmp = calloc(1, sizeof(Type));
    tmp->ty = PTR;
    tmp->ptr_to = type;
    type = tmp;
  }
  return type;
}

/*
 * stmt = expr ";"
 *      | "return" expr ";"
 *      | "if" "(" expr ")" stmt ("else" stmt)?
 *      | "while" "(" expr ")" stmt
 *      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
 *      | "{" stmt* "}"
 *      | prefix ident ("[" expr "]")* ";"
 */
Node *stmt() {
  Node *node;

  if(consume("{")) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->next = NULL;
    Node head = {.next = NULL};
    Node *cur = &head;
    while(!consume("}")) {
      cur->next = stmt();
      cur = cur->next;
    }
    cur->next = NULL;
    node->body = head.next;
    return node;
  }

  if(consume_kind(TK_IF)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_IF;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    if(consume_kind(TK_ELSE)) {
      node->elsebody = stmt();
    }
    return node;
  }

  if(consume_kind(TK_WHILE)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_WHILE;
    expect("(");
    node->cond = expr();
    expect(")");
    node->body = stmt();
    return node;
  }

  if(consume_kind(TK_FOR)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_FOR;
    expect("(");
    if(!consume(";")) {
      node->lhs = expr();
      expect(";");
    }

    if(!consume(";")) {
      node->cond = expr();
      expect(";");
    }

    if(!consume(")")) {
      node->rhs = expr();
      expect(")");
    }
    node->body = stmt();
    return node;
  }

  if(check_prefix()) {
    LVar *lvar = calloc(1, sizeof(LVar));
    lvar->type = prefix();
    Token *tok = consume_ident();
    if(!tok) error_at(token->str, "Identifier expected");
    if(find_lvar(tok)) error_at(tok->str, "Second declaration");
    lvar->next = nodes->func->locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    nodes->func->locals = lvar;

    while(consume("[")) {
      Type *tmp = calloc(1, sizeof(Type));
      tmp->ty = ARY;
      int count = compute_const_expr(expr());
      tmp->size = get_size(lvar->type) * count;
      tmp->ptr_to = lvar->type;
      lvar->type = tmp;
      expect("]");
    }

    lvar->offset = nodes->offset + get_size(lvar->type);
    nodes->offset = lvar->offset;

    expect(";");

    // XXX: { int x; } does not work
    // should add new node type ND_DECLARE
    return stmt();
  }

  if(consume_kind(TK_RETURN)) {
    node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
  }
  else {
    node = expr();
  }
  expect(";");
  return node;
}

/*
 * program = prefix ident "(" (prefix ident ",")* (prefix ident)? ")" block
 */
void program() {
  while(!at_eof()) {
    Type *type = prefix();
    Token *tok = consume_ident();
    if(!tok) error("Function definition starts with identifier\n");
    Function *func = calloc(1, sizeof(Function));
    func->locals = NULL;
    func->name = tok->str;
    func->len = tok->len;
    func->type = type;
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFUN;
    node->func = func;
    node->next = nodes;
    nodes = node;
    expect("(");
    if(consume(")"))
      func->params = NULL;
    else {
      LVar head = {.next = NULL};
      LVar *cur = &head;
      for(;;) {
        Type *type = prefix();
        Token *tok = consume_ident();
        if(!tok) error("Identifier expected");
        LVar *lvar = calloc(1, sizeof(LVar));
        lvar->next = NULL;
        lvar->name = tok->str;
        lvar->len = tok->len;
        lvar->type = type;
        lvar->offset = nodes->offset + get_size(type);
        node->offset = lvar->offset;
        cur->next = lvar;
        cur = cur->next;
        if(!consume(",")) break;
      }
      expect(")");
      func->locals = head.next;
      func->params = head.next;
    }
    func->body = stmt(); // XXX: allow only block
  }
}

