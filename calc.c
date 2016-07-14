#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

typedef enum {NUMBER, PLUS, MINUS, MULTIPLY, DIVIDE, MODULO, POWER, LEFT_PAREN,
              RIGHT_PAREN, EOL} type;

typedef union data {
  char *str;
  double num;
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

double getNumber(parser *parser) {
  char *buf = malloc(18 * sizeof(char));
  int i = 0;
  //cap numbers at 19 digits
  while (i < 10 && isdigit(*parser->p)) {
    buf[i] = *parser->p;
    parser->p++;
    i++;
  }

  if (*parser->p == '.') {
    buf[i] = '.';
    parser->p++;
    i++;
    while (isdigit(*parser->p)) {
      if (i < 17) { //accept infinitly long decimals, but cut off after 6 digits
        buf[i] = *parser->p;
      }
      parser->p++;
      i++;
    }
  }

  buf[i] = '\0';

  double ret;
  sscanf(buf, "%lf", &ret);

  return ret;
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
  case '%':
    lexeme->t = MODULO;
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
      lexeme->data.num = getNumber(parser);
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
         || check(parser, DIVIDE) || check(parser, MODULO) || check(parser,
                                                                    POWER);
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
  }else if (check(parser, MODULO)) {
    match(parser, MODULO);
  } else {
    match(parser, POWER);
  }
}

int isBinaryOperator(token *lexeme) {
  return lexeme->t == PLUS || lexeme->t == MINUS || lexeme->t == MULTIPLY
         || lexeme->t == DIVIDE || lexeme->t == MODULO || lexeme->t == POWER;
}

int isLeftAssociative(token *lexeme) {
  return lexeme->t == PLUS || lexeme->t == MINUS || lexeme->t == MULTIPLY
         || lexeme->t == DIVIDE || lexeme->t == MODULO;
}

int getPrecedence(token *lexeme) {
  if (lexeme->t == PLUS || lexeme->t == MINUS) {
    return 0;
  } else if (lexeme->t == MULTIPLY || lexeme->t == DIVIDE || lexeme->t ==
             MODULO) {
    return 1;
  } else {
    return 2;
  }
}

token *compute(token *lhs, token *operator, token *rhs) {
  token *newLexeme = malloc(sizeof(token));
  newLexeme->t = NUMBER;

  switch (*operator->data.str) {
  case '+':
    newLexeme->data.num = lhs->data.num + rhs->data.num;
    break;
  case '-':
    newLexeme->data.num = lhs->data.num - rhs->data.num;
    break;
  case '*':
    newLexeme->data.num = lhs->data.num * rhs->data.num;
    break;
  case '/':
    newLexeme->data.num = lhs->data.num / rhs->data.num;
    break;
  case '%':
    newLexeme->data.num = fmod(lhs->data.num, rhs->data.num);
    break;
  case '^':
    newLexeme->data.num = pow(lhs->data.num, rhs->data.num);
    break;
  default:
    printf("Error computing, unexpected operator: %s\n", operator->data.str);
    exit(1);
  }

  return newLexeme;
}

token *negate(token *lexeme) {
  lexeme->data.num *= -1;
  return lexeme;
}

token *atom(parser *parser) {
  token *result = NULL;

  if (check(parser, NUMBER)) {
    result = parser->currentLexeme;
    match(parser, NUMBER);
  } else if (check(parser, LEFT_PAREN)) {
    match(parser, LEFT_PAREN);
    result = expression(parser, 0);
    match(parser, RIGHT_PAREN);
  } else {
    match(parser, MINUS);
    result = negate(atom(parser));
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
  token *ret = expression(parser, 0);
  match(parser, EOL);
  return ret;
}

void printToken(token *lexeme) {
  if (lexeme->t == NUMBER) {
    //print integers without the extra 0's
    if (lexeme->data.num - floor(lexeme->data.num) < 0.000001) {
      printf("%d\n", (int)lexeme->data.num);
    } else {
      printf("%lf\n", lexeme->data.num);
    }
  } else {
    printf("%s\n", lexeme->data.str);
  }
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

    printToken(result);
  }
  return 0;
}
