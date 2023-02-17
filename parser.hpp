#ifndef __PARSER_HPP__
#define __PARSER_HPP__

#include <iostream>
#include <optional>
#include <vector>

enum class TokenType { LeftParen, RightParen, Plus, Minus, Multiply, Divide, Number, Space };

// Query -> ( Op SP Expr SP Expr OptExpr ) .
// Op -> + .
// Op -> - .
// Op -> * .
// Op -> / .
// Expr -> ( Op SP Expr SP Expr OptExpr ) .
// Expr -> NUMBER .
// OptExpr -> SP Expr OptExpr .
// OptExpr -> .

class Parser {
    std::vector<std::pair<TokenType, int>> tokens;
    std::vector<std::pair<TokenType, int>>::iterator it;

    std::string buffer;

    std::optional<int> rule_query();
    std::optional<int> rule_expr();
    std::optional<std::vector<int>> rule_optexpr();
    std::optional<int> rule_subexp();

    std::optional<int> do_operation(TokenType op, std::vector<int> results);

    bool rule_operator();
    bool check_rule(TokenType type);
    bool check_rule_advance(TokenType type);
    std::optional<std::pair<TokenType, int>> get_next();
    std::optional<std::pair<TokenType, int>> get_token();

   public:
    void lex(std::string str);
    std::optional<int> parse();
    Parser(std::string str);
};

#endif  // __PARSER_HPP__
