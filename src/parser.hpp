#pragma once

#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>
#include <variant>

#include <fstream>

#include "arena.hpp"
#include "tokenization.hpp"

struct NodeTermIntLit
{
    Token int_lit;
};

struct NodeTermIdent
{
    Token ident;
};

struct NodeExpr;

struct NodeBinExprAdd
{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

// struct NodeBinExprMult
// {
//     NodeExpr* lhs;
//     NodeExpr* rhs;
// };

struct NodeBinExpr
{
    NodeBinExprAdd* var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*> var;
};

struct NodeExpr
{
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtDox
{
    NodeExpr* expr;
};

struct NodeStmtStreetSign
{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt
{
    std::variant<NodeStmtDox*, NodeStmtStreetSign*> var;
};

struct NodeProg
{
    std::vector<NodeStmt*> stmts;
};

class Parser
{
    public:
        inline explicit Parser(std::vector<Token> tokens)
            : m_tokens(std::move(tokens))
            , m_allocator(1024 * 1024 * 4)  // 4 MB
        {
        }

        std::optional<NodeTerm*> parse_term()
        {
            if (auto int_lit = try_consume(1, TokenType::int_lit))
            {
                auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
                term_int_lit->int_lit = int_lit.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_int_lit;

                return term;
            }
            else if (auto ident = try_consume(1, TokenType::ident))
            {
                auto term_ident = m_allocator.alloc<NodeTermIdent>();
                term_ident->ident = ident.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_ident;
                
                return term;
            }
            else
            {
                return std::nullopt;
            }
        }

        std::optional<NodeExpr*> parse_expr()
        {
            if (auto term = parse_term())
            {
                if (try_consume(1, TokenType::plus).has_value())
                {
                    auto bin_expr = m_allocator.alloc<NodeBinExpr>();
                    auto bin_expr_add = m_allocator.alloc<NodeBinExprAdd>();
                    auto lhs_expr = m_allocator.alloc<NodeExpr>();
                    
                    lhs_expr->var = term.value();
                    bin_expr_add->lhs = lhs_expr;

                    if (auto rhs = parse_expr())
                    {
                        bin_expr_add->rhs = rhs.value();
                        bin_expr->var = bin_expr_add;

                        auto expr = m_allocator.alloc<NodeExpr>();
                        expr->var = bin_expr;

                        return expr;
                    }
                    else
                    {
                        std::cerr << "Unexpected Expression" << std::endl;
                        
                        exit(EXIT_FAILURE);
                    }
                }
                else
                {
                    auto expr = m_allocator.alloc<NodeExpr>();
                    expr->var = term.value();

                    return expr;
                }
            }
            else
            {
                return std::nullopt;
            }
        }

        std::optional<NodeStmt*> parse_stmt()
        {
            if (try_consume(1, TokenType::StreetSign))
            {
                auto stmt_StreetSign = m_allocator.alloc<NodeStmtStreetSign>();
                stmt_StreetSign->ident = try_consume(1, TokenType::ident, "Identifier Is Expected");

                try_consume(1, TokenType::equal, "Token '=' Is Expected");

                if (auto expr = parse_expr())
                {
                    stmt_StreetSign->expr = expr.value();
                }
                else
                {
                    std::cerr << "Invalid 'StreetSign' Expression" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::GayMan, "Token 'GayMan' Is Expected");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_StreetSign;

                return stmt;
            }
            else if (try_consume(1, TokenType::dox))
            {
                auto stmt_dox = m_allocator.alloc<NodeStmtDox>();
                
                if (auto node_expr = parse_expr())
                {
                    stmt_dox->expr = node_expr.value();
                }
                else
                {
                    std::cerr << "Invalid 'dox' Expression" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::GayMan, "Token 'GayMan' Is Expected");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_dox;

                return stmt;
            }
            else
            {
                return std::nullopt;
            }
        }

        std::optional<NodeProg> parse_prog()
        {
            NodeProg prog;

            while (peek(1).has_value())
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
        [[nodiscard]] std::optional<Token> peek(const uint8_t& offset) const
        {
            if (m_index + (offset - 1) >= m_tokens.size())
            {
                return std::nullopt;
            }
            else
            {
                return m_tokens.at(m_index + (offset - 1));
            }
        }
        
        inline Token consume()
        {
            return m_tokens.at(m_index++);
        }

        inline std::optional<Token> try_consume(const uint8_t& offset, const TokenType& type)
        {
            if (peek(offset).has_value() && peek(offset).value().type == type)
            {
                return consume();
            }
            else
            {
                return std::nullopt;
            }
        }

        inline Token try_consume(const uint8_t& offset, const TokenType& type, const std::string& err_msg)
        {
            if (peek(offset).has_value() && peek(offset).value().type == type)
            {
                return consume();
            }
            else
            {
                std::cerr << err_msg << std::endl;
                
                exit(EXIT_FAILURE);
            }
        }

        const std::vector<Token> m_tokens;
        size_t m_index = 0;

        ArenaAllocator m_allocator;
};