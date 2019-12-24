#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "seacc.h"

bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
         ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') ||
         (c == '_');
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while(*p) {
    if(isspace(*p)) {
      p++;
      continue;
    }

    if(*p == '+'
        || *p == '-'
        || *p == '*'
        || *p == '/'
        || *p == '('
        || *p == ')'
        || *p == '{'
        || *p == '}'
        || *p == ';'
        || *p == ','
        || *p == '&'
        || *p == '['
        || *p == ']') {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '<') {
      if(p+1 && *(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '>') {
      if(*(p+1) == '=') { // p+1 would not be NULL (texts end with \0)
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '=') {
      if(*(p+1) == '=') {
        cur = new_token(TK_RESERVED, cur, p, 2);
        p += 2;
        continue;
      }

      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if(*p == '!' && *(p+1) == '=') {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    // TODO: detect escape
    if(*p == '\'' && *(p+1)!='\0' && *(p+2)=='\'') {
      cur = new_token(TK_CHAR_LITERAL, cur, p, 3);
      cur->val = *(p+1);
      p+=3;
      continue;
    }

    if(isdigit(*p)) {
      int digitlen = 0;
      while(isdigit(*(p+digitlen))) {
        digitlen++;
      }
      cur = new_token(TK_NUM, cur, p, digitlen);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if(memcmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if(memcmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if(memcmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if(memcmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if(memcmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if(memcmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if(memcmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }

    if(memcmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_CHAR, cur, p, 4);
      p += 4;
      continue;
    }

    if(is_alnum(*p)) {
      int len;
      for(len=0; is_alnum(*(p+len)); len++);
      cur = new_token(TK_IDENT, cur, p, len);
      p += len;
      continue;
    }

    error("字句解析できません");
  }

  new_token(TK_EOF, cur, p, 1);
  return head.next;
}

