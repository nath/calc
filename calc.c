#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {NUMBER, OPERATOR, EOL} type;

typedef struct token {
  type t;
  char* value;
  struct token *next;
  struct token *prev;
} token;

typedef struct parser {
  char *p;
  int numLexemes;
  token *lexemes;
  token *currentLexeme;
} parser;

char *getNumber(parser *parser) {
  char *buf = malloc(11*sizeof(char));
  int i=0;
  //cap numbers at 10 digits
  while (i < 10 && isdigit(*parser->p)) {
    buf[i] = *parser->p;
    parser->p++;
    i++;
  }

  buf[i] = '\0';
  return buf;
}

token *lex(parser *parser) {
  while (isspace(*parser->p)) {
    parser->p++;
  }

  token *lexeme = malloc(sizeof(lexeme));
  lexeme->value = NULL;
  lexeme->next = NULL;

  if (*parser->p == '\0') {
    lexeme->t = EOL;
    parser->p++;
    return lexeme;
  }

  switch (*parser->p) {
    case '+': case '-' : case '*': case '/': case '^':
      lexeme->t = OPERATOR;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    default:
      if (isdigit(*parser->p)) {
        lexeme->t = NUMBER;
        lexeme->value = getNumber(parser);
        return lexeme;
      } else {
        printf("Illegal character: %c\n", *parser->p);
        exit(1);
      }
  }

  return NULL;
}

void initParser(parser *parser) {
  parser->p = malloc(1024*sizeof(char));
  parser->numLexemes = 0;
  parser->lexemes = NULL;
  parser->currentLexeme = NULL;
}

int check(parser *parser, type t) {
  return parser->currentLexeme->t == t;
}

void advance(parser *parser) {
  parser->currentLexeme->next = lex(parser);
  parser->currentLexeme->next->prev = parser->currentLexeme;
  parser->currentLexeme = parser->currentLexeme->next;
  parser->numLexemes++;
}

void matchNoAdvance(parser *parser, type t) {
  if (!check(parser, t)) {
    printf("illegal\n");
    exit(1);
  }
}

void match(parser *parser, type t) {
  matchNoAdvance(parser, t);
  advance(parser);
}

void expression(parser *parser) {
  match(parser, NUMBER);
  if (check(parser, OPERATOR)) {
    match(parser, OPERATOR);
    expression(parser);
  }
}

void parse(parser *parser) {
  parser->currentLexeme = lex(parser);
  parser->numLexemes++;
  parser->lexemes = parser->currentLexeme;
  parser->lexemes->prev = NULL;
  expression(parser);
}

int myPow(int x, int n) {
  //too lazy to link math lib and deal with doubles
  if (n == 0 || x == 1) {
    return 1;
  }
  int y = 1;
  while (n != 1) {
    if (n % 2) {
      y *= x;
      n = (n-1)/2;
    } else {
      n /= 2;
    }
    x *= x;
  }

  return x*y;
}

int main(int argc, char **argv) {
  parser *parser = malloc(sizeof(parser));
  initParser(parser);
  while (1) {
    printf("> ");
    fgets(parser->p, 1024, stdin);
    parse(parser);
    while (parser->lexemes->next->t != EOL) {
      token *pointer, *highest;
      pointer = (highest = parser->lexemes);
      int maxPrecedence = 0;
      while (pointer->t != EOL) {
        if (*pointer->value == '+' || *pointer->value == '-') {
          if (maxPrecedence < 1) {
            maxPrecedence = 1;
            highest = pointer;
          }
        }
        if (*pointer->value == '*' || *pointer->value == '/') {
          if (maxPrecedence < 2) {
            maxPrecedence = 2;
            highest = pointer;
          }
        }
        if (*pointer->value == '^') {
          if (maxPrecedence < 3) {
            maxPrecedence = 3;
            highest = pointer;
          }
        }
        pointer = pointer->next;
      }
      token *newLexeme = malloc(sizeof(token));
      newLexeme->t = NUMBER;
      newLexeme->value = malloc(11*sizeof(char));
      int op1 = atoi(highest->prev->value);
      int op2 = atoi(highest->next->value);
      switch (*highest->value) {
        case '+':
          snprintf(newLexeme->value, 11, "%d", op1+op2);
          break;
        case '-':
          snprintf(newLexeme->value, 11, "%d", op1-op2);
          break;
        case '*':
          snprintf(newLexeme->value, 11, "%d", op1*op2);
          break;
        case '/':
          snprintf(newLexeme->value, 11, "%d", op1/op2);
          break;
        case '^':
          snprintf(newLexeme->value, 11, "%d", myPow(op1, op2));
          break;
      }
      newLexeme->prev = highest->prev->prev;
      newLexeme->next = highest->next->next;
      if (highest->prev->prev == NULL) {
        parser->lexemes = newLexeme;
      } else {
        highest->prev->prev->next = newLexeme;
      }
      newLexeme->next->prev = newLexeme;
    }
    printf("Result is: %s\n", parser->lexemes->value);
  }
  return 0;
}
