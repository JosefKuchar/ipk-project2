#include "parser.hpp"

/**
 * Tokenize the input string
 * @param str The input string
 */
bool Parser::tokenize(std::string str) {
    // Remove previous tokens
    tokens.clear();
    // Iterate over characters in the string
    for (auto& c : str) {
        // If the character is a digit, add it to the buffer
        if (isdigit(c)) {
            buffer += c;
            continue;
        } else {
            // This means that number is finished and we can add it to the list of tokens
            if (!buffer.empty()) {
                tokens.push_back(std::make_pair(TokenType::Number, std::stoi(buffer)));
                buffer.clear();
            }
            // Other tokens
            switch (c) {
                case '(':
                    tokens.push_back(std::make_pair(TokenType::LeftParen, 0));
                    break;
                case ')':
                    tokens.push_back(std::make_pair(TokenType::RightParen, 0));
                    break;
                case '+':
                    tokens.push_back(std::make_pair(TokenType::Plus, 0));
                    break;
                case '-':
                    tokens.push_back(std::make_pair(TokenType::Minus, 0));
                    break;
                case '*':
                    tokens.push_back(std::make_pair(TokenType::Multiply, 0));
                    break;
                case '/':
                    tokens.push_back(std::make_pair(TokenType::Divide, 0));
                    break;
                case ' ':
                    tokens.push_back(std::make_pair(TokenType::Space, 0));
                    break;
                // Invalid character
                default:
                    return false;
            }
        }
    }
    // End token
    tokens.push_back(std::make_pair(TokenType::End, 0));
    return true;
}

/**
 * Perform given operation on operands
 * @param op The operation to perform
 * @param operands Operands
 * @return The result of the operation (or nullopt if operation is invalid)
 */
std::optional<int> Parser::do_operation(TokenType op, std::vector<int> operands) {
    int result = operands[0];

    for (int i = 1; i < operands.size(); i++) {
        switch (op) {
            case TokenType::Plus:
                result += operands[i];
                break;
            case TokenType::Minus:
                result -= operands[i];
                break;
            case TokenType::Multiply:
                result *= operands[i];
                break;
            case TokenType::Divide:
                // Division by zero
                if (operands[i] == 0) {
                    return std::nullopt;
                }
                result /= operands[i];
                break;
            default:
                // This should never happen
                return std::nullopt;
        }
    }
    return result;
}

/**
 * Get current token and increment the iterator
 */
std::optional<std::pair<TokenType, int>> Parser::get_next() {
    if (it == tokens.end()) {
        return std::nullopt;
    } else {
        return *(it++);  // Return the current token and increment the iterator
    }
}

/**
 * Check current token and increment the iterator
 */
bool Parser::check_rule_advance(TokenType type) {
    auto token = get_next();
    return token.has_value() && token.value().first == type;
}

/**
 * Check current token
 */
bool Parser::check_rule(TokenType type) {
    if (it == tokens.end()) {
        return false;
    } else {
        return it->first == type;
    }
}

/**
 * Query rule
 */
std::optional<int> Parser::rule_query() {
    // Every query has to start with a left parenthesis
    if (!check_rule_advance(TokenType::LeftParen)) {
        return std::nullopt;
    }
    auto result = rule_subexp();
    // Expresion has to properly end
    if (!check_rule_advance(TokenType::End)) {
        return std::nullopt;
    }
    return result;
}

/**
 * Expression rule
 */
std::optional<int> Parser::rule_expr() {
    // Check if the next token is a left parenthesis
    // That means that we have a sub-expression
    if (check_rule(TokenType::LeftParen)) {
        it++;
        return rule_subexp();
        // Otherwise we have a number
    } else if (check_rule(TokenType::Number)) {
        auto token = get_next();
        if (token.has_value()) {
            return token.value().second;
        } else {
            return std::nullopt;
        }
        // Invalid token
    } else {
        return std::nullopt;
    }
}

/**
 * Shared part between query and expression rule
 */
std::optional<int> Parser::rule_subexp() {
    // Results from all sub-expressions
    std::vector<int> results;
    // Check if the next token is an operator
    TokenType op = it->first;
    if (!rule_operator()) {
        return std::nullopt;
    }
    // Check if the next token is a space
    if (!check_rule_advance(TokenType::Space)) {
        return std::nullopt;
    }
    // Parse expression
    std::optional<int> expr = rule_expr();
    if (!expr) {
        return std::nullopt;
    }
    results.push_back(expr.value());
    // Check if the next token is a space
    if (!check_rule_advance(TokenType::Space)) {
        return std::nullopt;
    }
    // Parse expression
    expr = rule_expr();
    if (!expr) {
        return std::nullopt;
    }
    results.push_back(expr.value());
    // Parse optional expressions
    std::optional<std::vector<int>> optexpr = rule_optexpr();
    if (!optexpr) {
        return std::nullopt;
    }
    results.insert(results.end(), optexpr.value().begin(), optexpr.value().end());
    if (!check_rule_advance(TokenType::RightParen)) {
        return std::nullopt;
    }
    // Perform operation
    return do_operation(op, results);
}

/**
 * Optional expression rule
 */
std::optional<std::vector<int>> Parser::rule_optexpr() {
    // Results from other optional expressions
    std::vector<int> results;
    // If there is a space, then there is an optional expression
    if (check_rule(TokenType::Space)) {
        it++;
        // Parse expression
        std::optional<int> expr = rule_expr();
        if (!expr) {
            return std::nullopt;
        }
        // Add expression to results
        results.push_back(expr.value());
        // Parse next optional expression
        std::optional<std::vector<int>> optexpr = rule_optexpr();
        if (!optexpr) {
            return std::nullopt;
        }
        // Add results from next optional expressions to results
        results.insert(results.end(), optexpr.value().begin(), optexpr.value().end());
    }
    return results;
}

/**
 * Operator rule
 */
bool Parser::rule_operator() {
    // Get next token
    auto token = get_next();
    if (!token.has_value()) {
        return false;
    }
    // Check if the token is an operator
    switch (token.value().first) {
        case TokenType::Plus:
            return true;
        case TokenType::Minus:
            return true;
        case TokenType::Multiply:
            return true;
        case TokenType::Divide:
            return true;
        default:
            return false;
    }
}

std::optional<int> Parser::parse(std::string query) {
    // Tokenize query
    if (!tokenize(query)) {
        return std::nullopt;
    }
    // Reset iterator
    it = tokens.begin();
    // Parse query
    return rule_query();
}

Parser::Parser() {}
