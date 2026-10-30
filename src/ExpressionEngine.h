// ExpressionEngine.h — bracket check, infix→postfix, postfix evaluation
#pragma once
#include <string>
#include <vector>

struct BracketResult {
    bool ok;
    std::string message;          // human-readable error or "balanced"
};

struct ConversionResult {
    bool ok;
    std::vector<std::string> postfix;
    std::string message;
};

struct EvalResult {
    bool ok;
    double value;
    std::string message;
};

class ExpressionEngine {
public:
    // Part 2
    static BracketResult checkBrackets(const std::string& input);

    // Part 3: full validation + conversion
    static ConversionResult infixToPostfix(const std::string& input);

    // Part 4
    static EvalResult evaluatePostfix(const std::vector<std::string>& postfix);

    // Convenience
    static std::string joinTokens(const std::vector<std::string>& tokens);
};