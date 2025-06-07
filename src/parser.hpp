#pragma once

#include <iostream>

#include <sstream>
#include <vector>

#include <optional>
#include <variant>

#include <fstream>

#include "arena.hpp"
#include "tokenizer.hpp"

// Integer Literal Node
struct NodeTermIntLit
{
    Token int_lit;
};

// Identifier (variable) Node
struct NodeTermIdent
{
    Token ident;
};

struct NodeExpr; // Forward Declaration

// Parenthesized Expression Node
struct NodeTermParen
{
    NodeExpr* expr;
};

// Binary Operators
enum class BinOp
{
    Add,
    Sub,
    Mul,
    Div
};

// Binary Expression Node
struct NodeBinExpr
{
    BinOp op;
    NodeExpr* lhs;
    NodeExpr* rhs;
};

// Term Node
struct NodeTerm
{
    std::variant<NodeTermIntLit*, NodeTermIdent*, NodeTermParen*> var;
};

// Expression Node
struct NodeExpr
{
    std::variant<NodeTerm*, NodeBinExpr*> var;
};

// Return Statement Node
struct NodeStmtBeende
{
    NodeExpr* expr;
};

// Assignment Statement Node
struct NodeStmtBestimme
{
    Token ident;
    NodeExpr* expr;
};

struct NodeStmt; // Forward Declaration

// Block Scope Node
struct NodeScope
{
    std::vector<NodeStmt*> stmts;
};

// If Statement Node
struct NodeStmtFalls
{
    NodeExpr* expr;
    NodeScope* scope;
};

// Statement Node
struct NodeStmt
{
    std::variant<NodeStmtBeende*, NodeStmtBestimme*, NodeScope*, NodeStmtFalls*> var;
};

// Program Root Node
struct NodeProg
{
    std::vector<NodeStmt*> stmts;
};

class Parser
{
    public:
        explicit Parser(std::vector<Token> tokens)
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
            
            if (auto ident = try_consume(1, TokenType::ident))
            {
                auto term_ident = m_allocator.alloc<NodeTermIdent>();
                term_ident->ident = ident.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_ident;
                
                return term;
            }
            
            if (auto open_paren = try_consume(1, TokenType::open_paren))
            {
                auto expr = parse_expr();

                if (!expr.has_value())
                {
                    std::cerr << "Fehler: Unerwarteter Ausdruck" << std::endl;

                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::close_paren, "Fehler: Token ')' wird erwartet");

                auto term_paren = m_allocator.alloc<NodeTermParen>();
                term_paren->expr = expr.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_paren;

                return term;
            }
            
            return std::nullopt;
        }

        std::optional<NodeExpr*> parse_expr(const size_t min_prec = 0)
        {
            std::optional<NodeTerm*> term_lhs = parse_term();
        
            if (!term_lhs.has_value())
                return std::nullopt;
        
            auto expr_lhs = m_allocator.alloc<NodeExpr>();
            expr_lhs->var = term_lhs.value();
        
            while (true)
            {
                auto curr_tok = peek(1);
                auto prec = curr_tok ? bin_prec(curr_tok->type) : std::nullopt;
            
                if (!prec.has_value() || prec.value() < min_prec)
                    break;
            
                const auto [type, value] = consume();
                const size_t next_min_prec = prec.value() + 1;
            
                auto expr_rhs = parse_expr(next_min_prec);
            
                if (!expr_rhs.has_value())
                {
                    std::cerr << "Fehler: Ausdruck kann nicht geparst werden" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            
                auto bin_expr = m_allocator.alloc<NodeBinExpr>();
            
                bin_expr->lhs = expr_lhs;
            
                if (type == TokenType::plus)
                    bin_expr->op = BinOp::Add;

                else if (type == TokenType::minus)
                    bin_expr->op = BinOp::Sub;

                else if (type == TokenType::star)
                    bin_expr->op = BinOp::Mul;

                else if (type == TokenType::slash)
                    bin_expr->op = BinOp::Div;

                else
                {
                    std::cerr << "Fehler: Ungültiger Binäroperator" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            
                bin_expr->rhs = expr_rhs.value();
            
                expr_lhs = m_allocator.alloc<NodeExpr>();
                expr_lhs->var = bin_expr;
            }
        
            return expr_lhs;
        }

        std::optional<NodeScope*> parse_scope()
        {
            if (!try_consume(1, TokenType::open_curly).has_value())
            {
                return std::nullopt;
            }

            auto scope = m_allocator.alloc<NodeScope>();

            while (auto stmt = parse_stmt())
            {
                scope->stmts.push_back(stmt.value());
            }

            try_consume(1, TokenType::close_curly, "Fehler: Token '}' wird erwartet");

            return scope;
        }

        std::optional<NodeStmt*> parse_stmt()
        {
            if (try_consume(1, TokenType::Bestimme))
            {
                auto stmt_Bestimme = m_allocator.alloc<NodeStmtBestimme>();
                stmt_Bestimme->ident = try_consume(1, TokenType::ident, "Fehler: Bezeichner wird erwartet");

                try_consume(1, TokenType::als, "Fehler: Token 'als' wird erwartet");

                if (const auto expr = parse_expr())
                {
                    stmt_Bestimme->expr = expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger 'Bestimme'-Ausdruck" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::dot, "Fehler: Token '.' wird erwartet");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_Bestimme;

                return stmt;
            }
            
            if (try_consume(1, TokenType::Beende))
            {
                auto stmt_Beende = m_allocator.alloc<NodeStmtBeende>();
                
                if (const auto node_expr = parse_expr())
                {
                    stmt_Beende->expr = node_expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger 'Beende'-Ausdruck" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::dot, "Fehler: Token '.' wird erwartet");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_Beende;

                return stmt;
            }
            
            if (peek(1).has_value() && peek(1).value().type == TokenType::open_curly)
            {
                if (const auto scope = parse_scope())
                {
                    auto stmt = m_allocator.alloc<NodeStmt>();
                    stmt->var = scope.value();

                    return stmt;
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger Gültigkeitsbereich" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            }
            
            if (try_consume(1, TokenType::falls))
            {
                auto stmt_falls = m_allocator.alloc<NodeStmtFalls>();
                
                try_consume(1, TokenType::open_paren, "Fehler: Token '(' wird erwartet");
                
                if (const auto expr = parse_expr())
                {
                    stmt_falls->expr = expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültige Anweisung" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(1, TokenType::close_paren, "Fehler: Token ')' wird erwartet");
                try_consume(1, TokenType::dann, "Fehler: Token 'dann' wird erwartet");

                if (const auto scope = parse_scope())
                {
                    stmt_falls->scope = scope.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger Gültigkeitsbereich" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_falls;

                return stmt;
            }

            return std::nullopt;
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
                    std::cerr << "Fehler: Ungültige Anweisung" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            }

            return prog;
        }

    private:
        [[nodiscard]] std::optional<Token> peek(const size_t& offset) const
        {
            if (m_index + (offset - 1) >= m_tokens.size())
            {
                return std::nullopt;
            }
            
            return m_tokens.at(m_index + (offset - 1));
        }
        
        Token consume()
        {
            return m_tokens.at(m_index++);
        }

        std::optional<Token> try_consume(const size_t& offset, const TokenType& type)
        {
            if (peek(offset).has_value() && peek(offset).value().type == type)
            {
                return consume();
            }
            
            return std::nullopt;
        }

        Token try_consume(const size_t& offset, const TokenType& type, const std::string& err_msg)
        {
            if (peek(offset).has_value() && peek(offset).value().type == type)
            {
                return consume();
            }

            std::cerr << err_msg << std::endl;
                
            exit(EXIT_FAILURE);
        }

        const std::vector<Token> m_tokens;
        size_t m_index = 0;

        ArenaAllocator m_allocator;
};