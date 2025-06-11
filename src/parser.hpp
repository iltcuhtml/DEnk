#pragma once

#include <iostream>

#include <sstream>
#include <vector>

#include <optional>
#include <variant>

#include <fstream>

#include "ast.hpp"
#include "arena.hpp"
#include "tokenizer.hpp"

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
            if (auto int_lit = try_consume(TokenType::int_lit))
            {
                auto term_int_lit = m_allocator.alloc<NodeTermIntLit>();
                term_int_lit->int_lit = int_lit.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_int_lit;

                return term;
            }
            
            if (auto ident = try_consume(TokenType::ident))
            {
                auto term_ident = m_allocator.alloc<NodeTermIdent>();
                term_ident->ident = ident.value();

                auto term = m_allocator.alloc<NodeTerm>();
                term->var = term_ident;
                
                return term;
            }
            
            if (auto open_paren = try_consume(TokenType::open_paren))
            {
                auto expr = parse_expr();

                if (!expr.has_value())
                {
                    std::cerr << "Fehler: Unerwarteter Ausdruck" << std::endl;

                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::close_paren, "Fehler: Token ')' wird erwartet");

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
                auto curr_tok = peek();
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

                if (type == TokenType::plus || type == TokenType::minus || type == TokenType::star || type == TokenType::slash)
                {            
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
                else if (type == TokenType::gleich || type == TokenType::ungleich ||
                         type == TokenType::kleiner || type == TokenType::größer ||
                         type == TokenType::und || type == TokenType::oder || type == TokenType::nicht)
                {
                    auto logic_expr = m_allocator.alloc<NodeLogicExpr>();
                    
                    logic_expr->lhs = expr_lhs;

                    // TODO

                    // if (type == TokenType::)
                    //     logic_expr->op = LogicOp::;

                    // else if (type == TokenType::)
                    //     logic_expr->op = LogicOp::;

                    // else if (type == TokenType::)
                    //     logic_expr->op = LogicOp::;

                    // else if (type == TokenType::)
                    //     logic_expr->op = LogicOp::;

                    // else
                    // {
                    //     std::cerr << "Fehler: Logic_expr 에러 메시지 재작성 필요" << std::endl;

                    //     exit(EXIT_FAILURE);
                    // }
                
                    logic_expr->rhs = expr_rhs.value();
                
                    expr_lhs = m_allocator.alloc<NodeExpr>();
                    expr_lhs->var = logic_expr;
                }
            }
        
            return expr_lhs;
        }

        std::optional<NodeScope*> parse_scope()
        {
            if (!try_consume(TokenType::open_curly).has_value())
            {
                return std::nullopt;
            }

            auto scope = m_allocator.alloc<NodeScope>();

            while (auto stmt = parse_stmt())
            {
                scope->stmts.push_back(stmt.value());
            }

            try_consume(TokenType::close_curly, "Fehler: Token '}' wird erwartet");

            return scope;
        }

        std::optional<NodeStmt*> parse_stmt()
        {
            if (peek().has_value() && peek().value().type == TokenType::open_curly)
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

            if (try_consume(TokenType::Bestimme))
            {
                auto stmt_Bestimme = m_allocator.alloc<NodeStmtBestimme>();

                stmt_Bestimme->ident = try_consume(TokenType::ident, "Fehler: Bezeichner wird erwartet");

                try_consume(TokenType::als, "Fehler: Token 'als' wird erwartet");

                if (const auto expr = parse_expr())
                {
                    stmt_Bestimme->expr = expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger 'Bestimme'-Ausdruck" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::dot, "Fehler: Token '.' wird erwartet");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_Bestimme;

                return stmt;
            }

            if (try_consume(TokenType::Ändere))
            {
                auto stmt_Ändere = m_allocator.alloc<NodeStmtÄndere>();

                stmt_Ändere->ident = try_consume(TokenType::ident, "Fehler: Bezeichner wird erwartet");

                try_consume(TokenType::zu, "Fehler: Token 'zu' wird erwartet");

                if (const auto expr = parse_expr())
                {
                    stmt_Ändere->expr = expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger 'Ändere'-Ausdruck" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::dot, "Fehler: Token '.' wird erwartet");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_Ändere;

                return stmt;
            }
            
            if (try_consume(TokenType::Falls))
            {
                auto stmt_falls = m_allocator.alloc<NodeStmtFalls>();
                
                try_consume(TokenType::open_paren, "Fehler: Token '(' wird erwartet");
                
                if (const auto expr = parse_expr())
                {
                    stmt_falls->expr = expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültige Anweisung" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::close_paren, "Fehler: Token ')' wird erwartet");
                try_consume(TokenType::dann, "Fehler: Token 'dann' wird erwartet");

                if (const auto scope = parse_scope())
                {
                    stmt_falls->scope = scope.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger Gültigkeitsbereich" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                if (try_consume(TokenType::Sonst))
                {
                    if (const auto falls = parse_stmt())
                    {
                        stmt_falls->sonst = falls.value();
                    }
                    else
                    {
                        std::cerr << "Fehler: falls-sonst 에러 메시지 재작성 필요" << std::endl;

                        exit(EXIT_FAILURE);
                    }
                }
                else
                    stmt_falls->sonst = std::nullopt;

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_falls;

                return stmt;
            }

            if (try_consume(TokenType::Beende))
            {
                auto stmt_Beende = m_allocator.alloc<NodeStmtBeende>();

                try_consume(TokenType::mit, "Fehler: Token 'mit' wird erwartet");
                
                if (const auto node_expr = parse_expr())
                {
                    stmt_Beende->expr = node_expr.value();
                }
                else
                {
                    std::cerr << "Fehler: Ungültiger 'Beende'-Ausdruck" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }

                try_consume(TokenType::dot, "Fehler: Token '.' wird erwartet");

                auto stmt = m_allocator.alloc<NodeStmt>();
                stmt->var = stmt_Beende;

                return stmt;
            }

            return std::nullopt;
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
                    std::cerr << "Fehler: Ungültige Anweisung" << std::endl;
                    
                    exit(EXIT_FAILURE);
                }
            }

            return prog;
        }

    private:
        [[nodiscard]] std::optional<Token> peek(const size_t& offset = 0) const
        {
            if (m_index + offset >= m_tokens.size())
            {
                return std::nullopt;
            }
            
            return m_tokens.at(m_index + offset);
        }
        
        Token consume()
        {
            return m_tokens.at(m_index++);
        }

        std::optional<Token> try_consume(const TokenType& type)
        {
            if (peek().has_value() && peek().value().type == type)
            {
                return consume();
            }
            
            return std::nullopt;
        }

        std::optional<Token> try_consume(const size_t& offset, const TokenType& type)
        {
            if (peek(offset).has_value() && peek(offset).value().type == type)
            {
                return consume();
            }
            
            return std::nullopt;
        }

        Token try_consume(const TokenType& type, const std::string& err_msg)
        {
            if (peek().has_value() && peek().value().type == type)
            {
                return consume();
            }

            std::cerr << err_msg << std::endl;
                
            exit(EXIT_FAILURE);
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