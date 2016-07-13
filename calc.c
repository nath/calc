#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {NUMBER, PLUS, MINUS, MULTIPLY, DIVIDE, POWER, LEFT_PAREN,
              RIGHT_PAREN, EOL} type;

typedef union data {
  char *str;
  int i;
} data;

typedef struct token {
  type t;
  data data;
} token;

typedef struct parser {
  char *p;
  token *currentLexeme;
} parser;

token *expression(parser*, int);

int getNumber(parser *parser) {
  char *buf = malloc(20 * sizeof(char));
  int i = 0;
  //cap numbers at 19 digits
  while (i < 20 && isdigit(*parser->p)) {
    buf[i] = *parser->p;
    parser->p++;
    i++;
  }

  buf[i] = '\0';
  return atoi(buf);
}

token *lex(parser *parser) {
  while (isspace(*parser->p)) {
    parser->p++;
  }

  token *lexeme = malloc(sizeof(lexeme));

  if (*parser->p == '\0') {
    lexeme->t = EOL;
    parser->p++;
    return lexeme;
  }

  switch (*parser->p) {
  case '+':
    lexeme->t = PLUS;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case '-':
    lexeme->t = MINUS;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case '*':
    lexeme->t = MULTIPLY;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case '/':
    lexeme->t = DIVIDE;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case '^':
    lexeme->t = POWER;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case '(':
    lexeme->t = LEFT_PAREN;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  case ')':
    lexeme->t = RIGHT_PAREN;
    lexeme->data.str = malloc(sizeof(char));
    *lexeme->data.str = *parser->p;
    parser->p++;
    return lexeme;
  default:
    if (isdigit(*parser->p)) {
      lexeme->t = NUMBER;
      lexeme->data.i = getNumber(parser);
      return lexeme;
    } else {
      printf("Illegal character: %c\n", *parser->p);
      exit(1);
    }
  }

  return NULL;
}

int check(parser *parser, type t) {
  return parser->currentLexeme->t == t;
}

void advance(parser *parser) {
  parser->currentLexeme = lex(parser);
}

void matchNoAdvance(parser *parser, type t) {
  if (!check(parser, t)) {
    printf("illegal: wanted %d got a %d\n", t, parser->currentLexeme->t);
    exit(1);
  }
}

void match(parser *parser, type t) {
  matchNoAdvance(parser, t);
  advance(parser);
}

int operatorPending(parser *parser) {
  return check(parser, PLUS) || check(parser, MINUS) || check(parser, MULTIPLY)
         || check(parser, DIVIDE) || check(parser, POWER);
}

void operator(parser *parser) {
  if (check(parser, PLUS)) {
    match(parser, PLUS);
  } else if (check(parser, MINUS)) {
    match(parser, MINUS);
  } else if (check(parser, MULTIPLY)) {
    match(parser, MULTIPLY);
  } else if (check(parser, DIVIDE)) {
    match(parser, DIVIDE);
  } else {
    match(parser, POWER);
  }
}

int isBinaryOperator(token *lexeme) {
  return lexeme->t == PLUS || lexeme->t == MINUS || lexeme->t == MULTIPLY
         || lexeme->t == DIVIDE || lexeme->t == POWER;
}

int isLeftAssociative(token *lexeme) {
  return lexeme->t == PLUS || lexeme->t == MINUS || lexeme->t == MULTIPLY
         || lexeme->t == DIVIDE;
}

int getPrecedence(token *lexeme) {
  if (lexeme->t == PLUS || lexeme->t == MINUS) {
    return 0;
  } else if (lexeme->t == MULTIPLY || lexeme->t == DIVIDE) {
    return 1;
  } else {
    return 2;
  }
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
      n = (n - 1) / 2;
    } else {
      n /= 2;
    }
    x *= x;
  }

  return x * y;
}

token *compute(token *lhs, token *operator, token *rhs) {
  token *newLexeme = malloc(sizeof(token));
  newLexeme->t = NUMBER;

  switch (*operator->data.str) {
  case '+':
    newLexeme->data.i = lhs->data.i + rhs->data.i;
    break;
  case '-':
    newLexeme->data.i = lhs->data.i - rhs->data.i;
    break;
  case '*':
    newLexeme->data.i = lhs->data.i * rhs->data.i;
    break;
  case '/':
    newLexeme->data.i = lhs->data.i / rhs->data.i;
    break;
  case '^':
    newLexeme->data.i = myPow(lhs->data.i, rhs->data.i);
    break;
  default:
    printf("Error computing, unexpected operator: %s\n", operator->data.str);
    exit(1);
  }

  return newLexeme;
}

token *negate(token *lexeme) {
  lexeme->data.i *= -1;
  return lexeme;
}

token *atom(parser *parser) {
  token *result = parser->currentLexeme;
  if (check(parser, MINUS)) {
    match(parser, MINUS);
    return negate(atom(parser));
  }
  if (check(parser, NUMBER)) {
    match(parser, NUMBER);
  } else if (check(parser, LEFT_PAREN)) {
    match(parser, LEFT_PAREN);
    result = expression(parser, 0);
    match(parser, RIGHT_PAREN);
  }

  return result;
}

token *expression(parser *parser, int minPrecedence) {
  token *result = atom(parser);

  while (operatorPending(parser) &&
         isBinaryOperator(parser->currentLexeme) &&
         getPrecedence(parser->currentLexeme) >= minPrecedence) {
    int currPrecedence = getPrecedence(parser->currentLexeme);
    token *op = parser->currentLexeme;
    operator(parser);
    int nextMinPrecedence;
    if (isLeftAssociative(op)) {
      nextMinPrecedence = currPrecedence + 1;
    } else {
      nextMinPrecedence = currPrecedence;
    }
    token *rhs = expression(parser, nextMinPrecedence);
    result = compute(result, op, rhs);
  }

  return result;
}

token *parse(parser *parser) {
  parser->currentLexeme = lex(parser);
  return expression(parser, 0);
}

void initParser(parser *parser) {
  parser->p = malloc(1024 * sizeof(char));
  parser->currentLexeme = NULL;
}

int main(int argc, char **argv) {
  parser *parser = malloc(sizeof(parser));
  initParser(parser);
  while (1) {
    printf("> ");
    fgets(parser->p, 1024, stdin);
    token *result = parse(parser);

    printf("Result is: %d\n", result->data.i);
  }
  return 0;
}
