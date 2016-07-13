#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

typedef enum {NUMBER, PLUS, MINUS, MULTIPLY, DIVIDE, POWER, LEFT_PAREN, RIGHT_PAREN, EOL} type;

typedef struct token {
  type t;
  char* value;
} token;

typedef struct parser {
  char *p;
  token *currentLexeme;
} parser;

token *expression(parser*, int);

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

  if (*parser->p == '\0') {
    lexeme->t = EOL;
    parser->p++;
    return lexeme;
  }

  switch (*parser->p) {
    case '+':
      lexeme->t = PLUS;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case '-':
      lexeme->t = MINUS;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case '*':
      lexeme->t = MULTIPLY;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case '/':
      lexeme->t = DIVIDE;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case '^':
      lexeme->t = POWER;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case '(':
      lexeme->t = LEFT_PAREN;
      lexeme->value = malloc(sizeof(char));
      *lexeme->value = *parser->p;
      parser->p++;
      return lexeme;
    case ')':
      lexeme->t = RIGHT_PAREN;
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
  parser->currentLexeme = NULL;
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

token *negate(token *lexeme) {
  token *result = malloc(sizeof(token));
  result->t = NUMBER;
  result->value = malloc(11*sizeof(char));
  snprintf(result->value, 11, "%d", -1*atoi(lexeme->value));
  return result;
}

token *atom(parser *parser) {
  token *result = parser->currentLexeme;
  if (check(parser, MINUS)) {
    match(parser, MINUS);
    return negate(atom(parser));
  }
  if (check(parser, NUMBER)) {
    match(parser, NUMBER);
  } else if (check(parser, LEFT_PAREN)){
    match(parser, LEFT_PAREN);
    result = expression(parser, 0);
    match(parser, RIGHT_PAREN);
  }

  return result;
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
      n = (n-1)/2;
    } else {
      n /= 2;
    }
    x *= x;
  }

  return x*y;
}

token *compute(token *lhs, token *operator, token *rhs) {
  token *newLexeme = malloc(sizeof(token));
  newLexeme->t = NUMBER;
  newLexeme->value = malloc(11*sizeof(char));

  int op1 = atoi(lhs->value);
  int op2 = atoi(rhs->value);
  switch (*operator->value) {
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
    default:
      printf("Error computing, unexpected operator: %s\n", operator->value);
      exit(1);
  }

  return newLexeme;
}

token *expression(parser *parser, int minPrecedence) {
  token *result = atom(parser);

  while (operatorPending(parser) && (isBinaryOperator(parser->currentLexeme) && getPrecedence(parser->currentLexeme) >= minPrecedence)) {
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

int main(int argc, char **argv) {
  parser *parser = malloc(sizeof(parser));
  initParser(parser);
  while (1) {
    printf("> ");
    fgets(parser->p, 1024, stdin);
    token *result = parse(parser);

    printf("Result is: %s\n", result->value);
  }
  return 0;
}
