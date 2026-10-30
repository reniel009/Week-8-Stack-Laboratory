// main.cpp — Week 8 CC105, single-file build for Run Code (no .json needed)
#include "Stack.h"
#include "ExpressionEngine.h"

#include <cctype>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// =====================================================================
// ExpressionEngine implementation (moved here so one-file compile works)
// =====================================================================
namespace {

bool isOpening(char c) { return c == '(' || c == '[' || c == '{'; }
bool isClosing(char c) { return c == ')' || c == ']' || c == '}'; }

char matchingOpen(char close) {
    switch (close) {
        case ')': return '(';
        case ']': return '[';
        case '}': return '{';
        default:  return '\0';
    }
}

int precedence(const std::string& op) {
    if (op == "*" || op == "/") return 2;
    if (op == "+" || op == "-") return 1;
    return 0;
}

bool isOperatorToken(const std::string& t) {
    return t == "+" || t == "-" || t == "*" || t == "/";
}

// ---------- Part 3: infix → postfix with operand/operator state machine ----------
enum class State { ExpectOperand, ExpectOperator };

bool tokenize(const std::string& input,
              std::vector<std::string>& tokens,
              std::string& err) {
    std::size_t i = 0;
    State state = State::ExpectOperand;

    while (i < input.size()) {
        char c = input[i];
        if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }

        if (std::isdigit(static_cast<unsigned char>(c))) {
            if (state == State::ExpectOperator) { err = "Missing operator: two operands in a row"; return false; }
            std::size_t start = i;
            while (i < input.size() && std::isdigit(static_cast<unsigned char>(input[i]))) ++i;
            tokens.push_back(input.substr(start, i - start));
            state = State::ExpectOperator;
            continue;
        }
        // 'ExpectOperator' followed by '(' means the user wrote
        // something like 2(3+4). The handout rejects implicit
        // multiplication, so treat this as an error rather than
        // silently inserting a '*'.
        if (c == '(' || c == '[' || c == '{') {
            if (state == State::ExpectOperator) { err = "Implicit multiplication is not allowed"; return false; }
            tokens.push_back(std::string(1, c));
            state = State::ExpectOperand;
            ++i;
            continue;
        }

        if (c == ')' || c == ']' || c == '}') {
            if (state == State::ExpectOperand) { err = "Empty group or closing bracket where operand expected"; return false; }
            tokens.push_back(std::string(1, c));
            state = State::ExpectOperator;
            ++i;
            continue;
        }

        if (c == '+' || c == '-' || c == '*' || c == '/') {
            if (state == State::ExpectOperand) { err = "Operator where operand expected (unary signs unsupported)"; return false; }
            tokens.push_back(std::string(1, c));
            state = State::ExpectOperand;
            ++i;
            continue;
        }

        err = std::string("Unsupported character '") + c + "'";
        return false;
    }

    if (tokens.empty()) { err = "Empty input"; return false; }
    if (state == State::ExpectOperand) { err = "Expression ends while expecting an operand"; return false; }
    return true;
}

} // namespace

// ---------- Part 2: bracket-only pass ----------
BracketResult ExpressionEngine::checkBrackets(const std::string& input) {
    Stack<char> st;
    for (std::size_t i = 0; i < input.size(); ++i) {
        char c = input[i];
        if (isOpening(c)) {
            st.push(c);
        } else if (isClosing(c)) {
            char top;
            if (!st.peek(top))
                return {false, std::string("Unmatched closing bracket '") + c + "' at position " + std::to_string(i + 1)};
            // We must peek before popping so that when the pair is wrong,
            // we can report which opening bracket was on top. Popping
            // first would lose that information.
            if (top != matchingOpen(c)) {
                st.pop(top);
                return {false, std::string("Mismatched pair: expected '") + matchingOpen(c) + "' but found '" + top + "'"};
            }
            st.pop(top);
        }
    }
    if (!st.isEmpty()) {
        char top; st.peek(top);
        return {false, std::string("Unclosed opening bracket '") + top + "' remains"};
    }
    return {true, "balanced"};
}

ConversionResult ExpressionEngine::infixToPostfix(const std::string& input) {
    std::vector<std::string> tokens;
    std::string err;
    if (!tokenize(input, tokens, err)) return {false, {}, err};

    Stack<std::string> ops;
    std::vector<std::string> out;

    for (const auto& t : tokens) {
        if (std::isdigit(static_cast<unsigned char>(t[0]))) {
            out.push_back(t);
        } else if (t.size() == 1 && isOpening(t[0])) {
            ops.push(t);
        } else if (t.size() == 1 && isClosing(t[0])) {
            bool matched = false;
            // Pop when precedence(top) >= precedence(incoming). The '>='
            // (not '>') is what makes - and / left-associative:
            // 10 - 3 - 2 must convert to 10 3 - 2 -, not 10 3 2 - -.

            std::string top;
            while (ops.peek(top)) {
                if (top.size() == 1 && isOpening(top[0])) {
                    if (top[0] != matchingOpen(t[0])) return {false, {}, "Mismatched bracket during conversion"};
                    ops.pop(top);
                    matched = true;
                    break;
                }
                ops.pop(top);
                out.push_back(top);
            }
            if (!matched) return {false, {}, "Closing bracket with no matching opening"};
        } else {
            std::string top;
            while (ops.peek(top) && isOperatorToken(top) && precedence(top) >= precedence(t)) {
                ops.pop(top);
                out.push_back(top);
            }
            ops.push(t);
        }
    }

    std::string top;
    while (ops.peek(top)) {
        if (top.size() == 1 && isOpening(top[0])) return {false, {}, "Unclosed bracket remains on operator stack"};
        ops.pop(top);
        out.push_back(top);
    }
    return {true, out, "ok"};
}

// ---------- Part 4: postfix evaluation ----------
EvalResult ExpressionEngine::evaluatePostfix(const std::vector<std::string>& postfix) {
    Stack<double> values;
    for (const auto& t : postfix) {
        if (isOperatorToken(t)) {
            // Pop right first, then left. The top of the stack is the
            // most recently pushed number, which is the RIGHT operand
            // in postfix order. This matters for - and /:
            // 10 2 - must be 10 - 2 = 8, not 2 - 10 = -8.
            double right, left;
            if (!values.pop(right)) return {false, 0, "Missing right operand"};
            if (!values.pop(left))  return {false, 0, "Missing left operand"};

            double r = 0;
            if (t == "+") r = left + right;
            else if (t == "-") r = left - right;
            else if (t == "*") r = left * right;
            else {
                if (right == 0.0) return {false, 0, "Division by zero"};
                r = left / right;
            }
            if (!std::isfinite(r)) return {false, 0, "Non-finite result"};
            values.push(r);
        } else {
            try {
                std::size_t idx = 0;
                double v = std::stod(t, &idx);
                if (idx != t.size()) return {false, 0, "Bad numeric token: " + t};
                values.push(v);
            } catch (...) { return {false, 0, "Numeric conversion error: " + t}; }
        }
    }
    double result;
    if (!values.pop(result)) return {false, 0, "Empty expression"};
    if (!values.isEmpty())  return {false, 0, "Extra operands left on stack"};
    return {true, result, "ok"};
}

std::string ExpressionEngine::joinTokens(const std::vector<std::string>& tokens) {
    std::string s;
    for (std::size_t i = 0; i < tokens.size(); ++i) { if (i) s += ' '; s += tokens[i]; }
    return s;
}

// =====================================================================
// Menu
// =====================================================================
namespace {

void stackDemo() {
    Stack<int> s;
    int v;
    std::cout << "\n[Stack demo]\n";
    std::cout << "peek on empty: " << (s.peek(v) ? "value" : "false (no crash)") << "\n";
    std::cout << "pop  on empty: " << (s.pop(v)  ? "value" : "false (no crash)") << "\n";
    s.push(10); s.push(20);
    s.peek(v);
    std::cout << "after push 10,20  peek -> " << v << "  size=" << s.size() << "\n";
    s.push(30);
    std::cout << "after push 30      size=" << s.size() << "  (array grew)\n";
    std::cout << "pop sequence: ";
    while (s.pop(v)) std::cout << v << ' ';
    std::cout << "\nsize after pops: " << s.size() << "\n";
}

void convertAndEvaluate(const std::string& input) {
    auto br = ExpressionEngine::checkBrackets(input);
    std::cout << "\nInput: " << input << "\n";
    std::cout << "Brackets: " << (br.ok ? "balanced" : br.message) << "\n";
    if (!br.ok) return;

    auto cr = ExpressionEngine::infixToPostfix(input);
    if (!cr.ok) { std::cout << "Conversion error: " << cr.message << "\n"; return; }
    std::cout << "Postfix: " << ExpressionEngine::joinTokens(cr.postfix) << "\n";

    auto er = ExpressionEngine::evaluatePostfix(cr.postfix);
    if (!er.ok) { std::cout << "Evaluation error: " << er.message << "\n"; return; }
    std::cout << "Result: " << er.value << "\n";
}

} // namespace

int main() {
    while (true) {
        std::cout << "\n===================================\n"
                  << "   CC105 Week 8: Stack Evaluator"
                  << "\n===================================\n"
                  << "1. Check brackets\n"
                  << "2. Convert + evaluate expression\n"
                  << "3. Run stack demonstration\n"
                  << "0. Exit\n"
                  << "Choice: ";
        std::string line;
        if (!std::getline(std::cin, line)) return 0;
        if (line.empty()) continue;
        char c = line[0];

        if (c == '0') { std::cout << "Bye.\n"; break; }
        if (c == '1' || c == '2') {
            std::cout << "\nExpression: ";
            std::string expr;
            std::getline(std::cin, expr);
            if (c == '1') {
                auto br = ExpressionEngine::checkBrackets(expr);
                std::cout << "\nBrackets: " << (br.ok ? "balanced" : br.message) << "\n";
            } else {
                convertAndEvaluate(expr);
            }
        } else if (c == '3') {
            stackDemo();
        } else {
            std::cout << "\nInvalid choice. Please enter 0-3.\n";
        }
    }
    return 0;
}