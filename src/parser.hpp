#pragma once

#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

#include "tokenization.hpp"

struct NodeExpr
{
    Token int_lit;
};

struct NodeDox
{
    NodeExpr expr;
};

class Parser
{
    public:
        inline explicit Parser(std::vector<Token> tokens) : m_tokens(std::move(tokens)) {}

        std::optional<NodeExpr> parse_expr()
        {
            if (peek().has_value() && peek().value().type == TokenType::int_lit)
            {
                return NodeExpr{ .int_lit = consume() };
            }
            else
            {
                return {};
            }
        }

        std::optional<NodeDox> parse()
        {
            std::optional<NodeDox> dox_node;

            while (peek().has_value())
            {
                if (peek().value().type == TokenType::dox)
                {
                    consume();

                    if (auto node_expr = parse_expr())
                    {
                        dox_node = NodeDox{ .expr = node_expr.value() };
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
                        std::cerr << "Invalid Expression" << std::endl;
                        
                        exit(EXIT_FAILURE);
                    }
                }
            }

            m_index = 0;

            return dox_node;
        }

    private:
        [[nodiscard]] std::optional<Token> peek(int ahead = 1) const
        {
            if (m_index + ahead > m_tokens.size())
            {
                return {};
            }
            else
            {
                return m_tokens.at(m_index);
            }
        }
        
        inline Token consume()
        {
            return m_tokens.at(m_index++);
        }

        const std::vector<Token> m_tokens;
        size_t m_index = 0;
};