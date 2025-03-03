#pragma once

#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>
#include <variant>

#include <fstream>

#include "tokenization.hpp"

struct NodeExprIntLit
{
    Token int_lit;
};

struct NodeExprIdent
{
    Token ident;
};

struct NodeExpr
{
    std::variant<NodeExprIntLit, NodeExprIdent> var;
};

struct NodeStmtDox
{
    NodeExpr expr;
};

struct NodeStmtStreetSign
{
    Token ident;
    NodeExpr expr;
};

struct NodeStmt
{
    std::variant<NodeStmtDox, NodeStmtStreetSign> var;
};

struct NodeProg
{
    std::vector<NodeStmt> stmts;
};

class Parser
{
    public:
        inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

        std::optional<NodeExpr> parse_expr()
        {
            if (peek().has_value() && peek().value().type == TokenType::int_lit)
            {
                return NodeExpr { .var = NodeExprIntLit { .int_lit = consume() } };
            }
            else if (peek().has_value() && peek().value().type == TokenType::ident)
            {
                return NodeExpr { .var = NodeExprIdent {.ident = consume() } };
            }
            else
            {
                return {};
            }
        }

        std::optional<NodeStmt> parse_stmt()
        {
            if (peek().has_value() && peek().value().type == TokenType::StreetSign
                 && peek(1).has_value() && peek(1).value().type == TokenType::ident
                 && peek(2).has_value() && peek(2).value().type == TokenType::equal)
            {
                consume();
                NodeStmtStreetSign stmt_StreetSign;
                consume();

                if (auto expr = parse_expr())
                {
                    stmt_StreetSign = NodeStmtStreetSign { .ident = consume(), .expr = expr.value() };
                }
                else
                {
                    std::cerr << "Invalid Expression" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                if (peek().has_value() && peek().value().type == TokenType::GayMan)
                {
                    consume();
                }
                else
                {
                    std::cerr << "Token 'GayMan' Is Expected" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                return NodeStmt { .var = stmt_StreetSign };
            }
            else if (peek().value().type == TokenType::dox)
            {
                consume();

                NodeStmtDox stmt_dox;
                
                if (auto node_expr = parse_expr())
                {
                    stmt_dox = NodeStmtDox { .expr = node_expr.value() };
                }
                else
                {
                    std::cerr << "Invalid Expression" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                if (peek().has_value() && peek().value().type == TokenType::GayMan)
                {
                    consume();
                }
                else
                {
                    std::cerr << "Token 'GayMan' Is Expected" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                return NodeStmt { .var = stmt_dox };
            }
            else
            {
                return {};
            }
        }

        std::optional<NodeProg> parse_prog()
        {
            NodeProg prog;

            while (peek().has_value())
            {
                if (auto stmt = parse_stmt())
                {
                    prog.stmts.push_back(stmt.value());
                }
                else
                {
                    std::cerr << "Invalid Statement" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            }

            return prog;
        }

    private:
        [[nodiscard]] std::optional<Token> peek(int offset = 0) const
        {
            if (m_index + offset >= m_tokens.size())
            {
                return {};
            }
            else
            {
                return m_tokens.at(m_index + offset);
            }
        }
        
        inline Token consume()
        {
            return m_tokens.at(m_index++);
        }

        const std::vector<Token> m_tokens;
        size_t m_index = 0;
};