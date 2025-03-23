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

struct NodeTermParen
{
    NodeExpr* expr;
};

struct NodeBinExprAdd
{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprSub
{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprMul
{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExprDiv
{
    NodeExpr* lhs;
    NodeExpr* rhs;
};

struct NodeBinExpr
{
    std::variant<NodeBinExprAdd*, NodeBinExprSub*, NodeBinExprMul*, NodeBinExprDiv*> var;
};

struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

struct NodeExpr
{
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

struct NodeStmtDox
{
    NodeExpr* expr;
};

struct NodeStmtBigGuy
{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt;

struct NodeScope
{
    std::vector<NodeStmt*> stmts;
};

struct NodeStmt
{
    std::variant<NodeStmtDox*, NodeStmtBigGuy*, NodeScope*> var;
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
            else if (auto open_paren = try_consume(1, TokenType::open_paren))
            {
                auto expr = parse_expr();

                if (!expr.has_value())
                {
                    std::cerr << "Unexpected Expression" << std::endl;

                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::close_paren, "Token ')' Is Expected");

                auto term_paren = m_allocator.alloc<NodeTermParen>();
                term_paren->expr = expr.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_paren;

                return term;
            }
            else
            {
                return std::nullopt;
            }
        }

        std::optional<NodeExpr*> parse_expr(uint8_t min_prec = 0)
        {
            std::optional<NodeTerm*> term_lhs = parse_term();

            if (!term_lhs.has_value())
            {
                return std::nullopt;
            }

            auto expr_lhs = m_allocator.alloc<NodeExpr>();
            expr_lhs->var = term_lhs.value();
            
            while (true)
            {
                std::optional<Token> curr_tok = peek(1);
                std::optional<uint8_t> prec;

                if (curr_tok.has_value())
                {
                    prec = bin_prec(curr_tok->type);

                    if (!prec.has_value() || prec < min_prec)
                    {
                        break;
                    }
                }
                else
                {
                    break;
                }
                
                Token op = consume();

                uint8_t next_min_prec = prec.value() + 1;

                auto expr_rhs = parse_expr(next_min_prec);

                if (!expr_rhs.has_value())
                {
                    std::cerr << "Unable To Parse Expression" << std::endl;

                    exit(EXIT_FAILURE);
                }

                auto expr = m_allocator.alloc<NodeBinExpr>();
                auto expr_lhs2 = m_allocator.alloc<NodeExpr>();
                
                if (op.type == TokenType::plus)
                {
                    auto add = m_allocator.alloc<NodeBinExprAdd>();
                    
                    expr_lhs2->var = expr_lhs->var;

                    add->lhs = expr_lhs2;
                    add->rhs = expr_rhs.value();

                    expr->var = add;
                }
                else if (op.type == TokenType::minus)
                {
                    auto sub = m_allocator.alloc<NodeBinExprSub>();
                    
                    expr_lhs2->var = expr_lhs->var;

                    sub->lhs = expr_lhs2;
                    sub->rhs = expr_rhs.value();

                    expr->var = sub;
                }
                else if (op.type == TokenType::star)
                {
                    auto mult = m_allocator.alloc<NodeBinExprMul>();

                    expr_lhs2->var = expr_lhs->var;

                    mult->lhs = expr_lhs2;
                    mult->rhs = expr_rhs.value();

                    expr->var = mult;
                }
                else if (op.type == TokenType::slash)
                {
                    auto div = m_allocator.alloc<NodeBinExprDiv>();
                    
                    expr_lhs2->var = expr_lhs->var;

                    div->lhs = expr_lhs2;
                    div->rhs = expr_rhs.value();

                    expr->var = div;
                }

                expr_lhs->var = expr;
            }

            return expr_lhs;
        }

        std::optional<NodeStmt*> parse_stmt()
        {
            if (try_consume(1, TokenType::BigGuy))
            {
                auto stmt_StreetSign = m_allocator.alloc<NodeStmtBigGuy>();
                stmt_StreetSign->ident = try_consume(1, TokenType::ident, "Identifier Is Expected");

                try_consume(1, TokenType::equal, "Token '=' Is Expected");

                if (auto expr = parse_expr())
                {
                    stmt_StreetSign->expr = expr.value();
                }
                else
                {
                    std::cerr << "Invalid 'BigGuy' Expression" << std::endl;
                    
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
            else if (auto open_curly = try_consume(1, TokenType::open_curly))
            {
                auto scope = m_allocator.alloc<NodeScope>();

                while (auto stmt = parse_stmt())
                {
                    scope->stmts.push_back(stmt.value());
                }

                try_consume(1, TokenType::close_curly, "Token '}' Is Expected");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = scope;

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