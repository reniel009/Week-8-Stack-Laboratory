# CC105 Week 8: Stack Laboratory

A C++ console application that implements a dynamic-array Stack ADT and uses it to check bracket balance, convert infix expressions to postfix, and evaluate postfix expressions.

## Member

| Name                      | Section   |
|---------------------------|-----------|
| Oliquino, Reniel Dave R.  | 2.4 BSIT  |

---

## Compiler Version and Tools

- **Compiler:** g++ (Rev8, Built by MSYS2 project) 15.2.0
- **Standard:** C++17 (`-std=c++17`)
- **IDE** VS Code or Code::Blocks

---

## Supported Syntax

- **Operands:** non-negative integers (multi-digit allowed)
- **Operators:** `+`, `-`, `*`, `/`
- **Brackets:** `()`, `[]`, `{}` — must match and nest correctly
- **Whitespace:** ignored between tokens

Example valid inputs: `{2+(3*4)}`, `(100+50)*2`, `20-6-3`, `{{[2+3]*(4+1)}}`, `7/2+1/2`

---

## Rejected inputs
Letters, decimals, unary signs, implicit multiplication (`2(3+4)`), empty groups,
malformed expressions, division by zero, and non-finite results.

---

## Limitations

1. No unary operators (`-5+2` rejected)
2. No implicit multiplication (`2(3)` rejected)
3. No floating-point literals (`4.5` rejected)
4. No variables or functions
5. Results stored as `double`; operands parsed as integers
6. Stack copy is disabled (move-only) to prevent shallow-copy bugs

---

## Algorithm Explanation

**Stack (`Stack.h`)** — Dynamic array; `push` doubles capacity when full; `pop`/`peek` return `false` on empty (no crash); copy deleted, move provided via `swap`. Amortized O(1) per operation.

**Bracket check** — Push opening brackets; on a closing bracket, compare with the top (empty stack or type mismatch → error). If the stack is non-empty at the end → unclosed bracket. O(n).

**Infix → postfix** — Two phases: (1) tokenizer state machine (`ExpectOperand`/`ExpectOperator`) rejects malformed input early; (2) shunting-yard pops operators with precedence ≥ the incoming one onto the output, treating brackets as barriers. O(n).

**Postfix evaluation** — Push operands; on an operator, pop right then left, compute, and push the result. Division-by-zero and non-finite results are errors. Exactly one value must remain at the end. O(n).

---

## Repository Structure

CC105_Week8_Section_GroupName/
├── screenshots/
│   ├── bracket error mismatached.png
│   ├── bracket error unclosed.png
│   ├── correct expression.png
│   └── division by zero.png
|   └── empty_stack handling.png
├── tests/
│   └── Test Report _ CC105 Week 8 Laboratory.pdf
├── src/
│   ├── ExpressionEngine.h     
│   ├── main.cpp
│   └── stack.h
└── README.md