#calc
##A kinda sucky (but ever improving) cli calculator

calc is a weird amalgamation of a recursive descent parser for maybe the world's simplest grammar and a calculator.

It only supports the operations +, - (subtraction and unary negation), *, /, %, and ^, for which it respects order of operations and associativity, which can be superseded with parentheses.

It uses precedence climbing to evaluate expressions.

calc stores all numbers in doubles, and does nothing about overflow.

calc is unique amongst all other calculators, including physical ones, in that it is the best calculator ever, even when including the above deficiencies. This is primarily due to the fact that I made it and I'm the best.
