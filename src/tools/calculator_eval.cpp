// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"

namespace {
    using Kind = CalcToken::Kind;

    // Return the precedence of the operator kind. Higher numbers mean higher precedence.
    int precedence(Kind k) {
        switch (k) {
            case Kind::Plus:
            case Kind::Minus:
                return 1; // Lowest precedence for addition and subtraction
            case Kind::Times:
            case Kind::Divide:
                return 2; // Higher precedence for multiplication and division
            default:
                return 0; // Parentheses and numbers have no precedence
        }
    }

    // Apply the operator at the top of the operators stack to the top two numbers on the numbers stack.
    bool apply_operator(std::vector<double>& numbers, std::vector<Kind>& operators, std::string* error) {
        if (numbers.size() < 2 || operators.empty()) {
            *error = "insufficient operands or operators";
            return false;
        }

        double right = numbers.back(); numbers.pop_back();
        double left = numbers.back(); numbers.pop_back();
        Kind op = operators.back(); operators.pop_back();

        switch (op) {
            case Kind::Plus:
                numbers.push_back(left + right);
                break;
            case Kind::Minus:
                numbers.push_back(left - right);
                break;
            case Kind::Times:
                numbers.push_back(left * right);
                break;
            case Kind::Divide:
                if (right == 0) {
                    *error = "division by zero";
                    return false;
                }
                numbers.push_back(left / right);
                break;
            default:
                *error = "invalid operator";
                return false;
        }
        return true;
    }
}

// Evaluate a well-formed token list with two stacks (one for numbers, one for operators and '(').
bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    std::vector<double> numbers; // Stack for numbers
    std::vector<Kind> operators; // Stack for operators and parentheses

    for (const auto& token : tokens) {
        switch (token.kind) {
            case Kind::Number:
                numbers.push_back(token.value);
                break;
            case Kind::Plus:
            case Kind::Minus:
            case Kind::Times:
            case Kind::Divide:
                while (!operators.empty() && operators.back() != Kind::LParen &&
                       precedence(operators.back()) >= precedence(token.kind)) {
                    if (!apply_operator(numbers, operators, error)) return false;
                }
                operators.push_back(token.kind);
                break;
            case Kind::LParen:
                operators.push_back(token.kind);
                break;
            case Kind::RParen:
                while (!operators.empty() && operators.back() != Kind::LParen) {
                    if (!apply_operator(numbers, operators, error)) return false;
                }
                if (operators.empty() || operators.back() != Kind::LParen) {
                    *error = "mismatched parentheses";
                    return false;
                }
                operators.pop_back(); // Pop the '('
                break;

                // Handle unexpected token kinds
                default:
                *error = "invalid token kind";
                return false;
        }
    }
    
    // After processing all tokens, apply remaining operators
    while (!operators.empty()) {
        if (operators.back() == Kind::LParen) {
            *error = "mismatched parentheses";
            return false;
        }
        if (!apply_operator(numbers, operators, error)) return false;       // Apply remaining operators
    }

    // At this point, there should be exactly one number left on the stack
    if (numbers.size() != 1) {
        *error = "invalid expression";
        return false;
    }

    *out = numbers.back();  // Set the output to the final result
    return true;
}
