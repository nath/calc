#calc
##A kinda sucky (but ever improving) cli calculator

calc is a weird amalgamation of a recursive descent parser for maybe the world's simplest grammar and a calculator.

It only uses positive integers, and has the operations +, -, *, /, and ^ (which uses exponentiation by squaring).

It doesn't support parentheses (but that's coming next!).

It also doesn't build a parse tree, but instead builds a parse linked list™, which I then just loop over and evaluate the highest precedence operator until there's only a single number left. This means it has a time complexity of O(n^2), which probably should be just O(n).

calc is unique amongst all other calculators, including physical ones, in that it is the best calculator ever, even when including the above deficiencies. This is primarily due to the fact that I made it and I'm the best.
