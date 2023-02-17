#include "parser.hpp"

void Parser::lex(std::string str) {
    for (auto& c : str) {
        if (isdigit(c)) {
            buffer += c;
            continue;
        } else {
            if (!buffer.empty()) {
                tokens.push_back(std::make_pair(TokenType::Number, std::stoi(buffer)));
                buffer.clear();
            }

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
                default:
                    std::cout << "Error" << std::endl;
                    break;
            }
        }
    }
}

std::optional<int> Parser::do_operation(TokenType op, std::vector<int> results) {
    int result = results[0];

    for (int i = 1; i < results.size(); i++) {
        switch (op) {
            case TokenType::Plus:
                result += results[i];
                break;
            case TokenType::Minus:
                result -= results[i];
                break;
            case TokenType::Multiply:
                result *= results[i];
                break;
            case TokenType::Divide:
                if (results[i] == 0) {
                    return std::nullopt;
                }
                result /= results[i];
                break;
            default:
                return std::nullopt;
        }
    }
    return result;
}

std::optional<std::pair<TokenType, int>> Parser::get_next() {
    if (it == tokens.end()) {
        return std::nullopt;
    } else {
        return *(it++);  // Return the current token and increment the iterator
    }
}

std::optional<std::pair<TokenType, int>> Parser::get_token() {
    if (it == tokens.end()) {
        return std::nullopt;
    } else {
        return *it;
    }
}

bool Parser::check_rule_advance(TokenType type) {
    auto token = get_next();
    if (token.has_value() && token.value().first == type) {
        return true;
    } else {
        return false;
    }
}

bool Parser::check_rule(TokenType type) {
    auto token = get_token();
    if (token.has_value() && token.value().first == type) {
        return true;
    } else {
        return false;
    }
}

std::optional<int> Parser::rule_query() {
    std::vector<int> results;
    if (!check_rule_advance(TokenType::LeftParen)) {
        return std::nullopt;
    }
    return rule_subexp();
}

std::optional<int> Parser::rule_expr() {
    // Check if the next token is a left parenthesis
    if (check_rule(TokenType::LeftParen)) {
        it++;
        return rule_subexp();
    } else if (check_rule(TokenType::Number)) {
        auto token = get_next();
        if (token.has_value()) {
            return token.value().second;
        } else {
            return std::nullopt;
        }
    } else {
        return std::nullopt;
    }
}

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

std::optional<int> Parser::parse() {
    // Reset iterator
    it = tokens.begin();
    // Parse query
    return rule_query();
}

Parser::Parser(std::string str) {
    // Tokenize string
    lex(str);
}
