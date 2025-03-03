#pragma once

#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

#include "tokenization.hpp"
#include "parser.hpp"

class Generator
{
    public:
        inline explicit Generator(NodeProg prog) : m_prog(std::move(prog)) {}

        void gen_expr(const NodeExpr& expr)
        {
            struct ExprVisitor
            {
                Generator* gen;

                void operator()(const NodeExprIntLit& expr_int_lit) const
                {
                    gen->m_output << "    mov rax, " << expr_int_lit.int_lit.value.value() << "\n";
                    gen->m_output << "    push rax\n";
                }

                // void operator()(const NodeExprIdent& expr_ident) const
                // {
                //     // ;
                // }
            };
            
            ExprVisitor visitor { .gen = this };
            std::visit(visitor, expr.var);
        }
        
        void gen_stmt(const NodeStmt& stmt)
        {
            struct StmtVisitor
            {
                Generator* gen;

                void operator()(const NodeStmtDox& stmt_dox) const
                {
                    gen->gen_expr(stmt_dox.expr);

                    gen->m_output << "    ; exit\n";
                    gen->m_output << "    pop rcx\n";
                    gen->m_output << "    call ExitProcess\n";
                    gen->m_output << "    ; /exit\n\n";
                }

                // void operator()(const NodeStmtStreetSign& stmt_StreetSign) const
                // {
                //     // ;
                // }
            };
            
            StmtVisitor visitor { .gen = this };
            std::visit(visitor, stmt.var);
        }

        [[nodiscard]] std::string gen_prog()
        {            
            m_output << "section .text\n";
            m_output << "    global main\n\n";
            
            m_output << "    extern printf\n";
            m_output << "    extern ExitProcess\n\n";
            
            m_output << "main:\n";
            m_output << "    sub rsp, 40\n\n";

            for (const NodeStmt& stmt : m_prog.stmts)
            {
                gen_stmt(stmt);
            }

            m_output << "    move rcx, 0\n";
            m_output << "    call ExitProcess\n\n";

            m_output << "    add rsp, 40\n";
            m_output << "    ret";

            return m_output.str();
        }
    
    private:
        const NodeProg m_prog;
        std::stringstream m_output;
};