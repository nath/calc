#calc
##A kinda sucky (but ever improving) cli calculator

calc is a weird amalgamation of a recursive descent parser for maybe the world's simplest grammar and a calculator.

It has the operations +, -, *, /, and ^ (which uses exponentiation by squaring), for which it respects order of operations and associativity, which can be superseded with parentheses.

It uses precedence climbing to evaluate expressions.

Currently calc lacks a unary negation operator, and as a result entering negative numbers requires parentheses and subtraction (probably from 0). calc also only supports integers and integer math.

calc is unique amongst all other calculators, including physical ones, in that it is the best calculator ever, even when including the above deficiencies. This is primarily due to the fact that I made it and I'm the best.
