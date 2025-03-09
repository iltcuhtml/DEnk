#pragma once

#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>
#include <unordered_map>

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
                    gen->push("rax");
                }

                void operator()(const NodeExprIdent& expr_ident) const
                {
                    if (!gen->m_vars.contains(expr_ident.ident.value.value()))
                    {
                        std::cerr << "Undeclared Identifier : " << expr_ident.ident.value.value() << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    const auto& var = gen->m_vars.at(expr_ident.ident.value.value());

                    std::stringstream offset;
                    offset << "QWORD [rsp + " << (gen->m_stack_size - var.stack_loc - 1) * 8 << "]\n";

                    gen->push(offset.str());
                }
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
                    gen->pop("rcx");
                    gen->m_output << "    call ExitProcess\n";
                    gen->m_output << "    ; /exit\n\n";
                }

                void operator()(const NodeStmtStreetSign& stmt_StreetSign) const
                {
                    if (gen->m_vars.contains(stmt_StreetSign.ident.value.value()))
                    {
                        std::cerr << "Identifier Is Already Used : " << stmt_StreetSign.ident.value.value() << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    gen->m_vars.insert({ stmt_StreetSign.ident.value.value(), Var { .stack_loc = gen->m_stack_size } });
                    gen->gen_expr(stmt_StreetSign.expr);
                }
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
            m_output << "    push rbp\n";
            m_output << "    mov rbp, rsp\n";
            m_output << "    sub rsp, 16\n\n";

            for (const NodeStmt& stmt : m_prog.stmts)
            {
                gen_stmt(stmt);
            }

            m_output << "    mov rcx, 0\n";
            m_output << "    call ExitProcess";

            return m_output.str();
        }
    
    private:
        void push(const std::string& reg)
        {
            m_output << "    push " << reg << "\n";
            m_stack_size++;
        }

        void pop(const std::string& reg)
        {
            m_output << "    pop " << reg << "\n";
            m_stack_size--;
        }

        struct Var
        {
            size_t stack_loc;
        };

        const NodeProg m_prog;
        std::stringstream m_output;

        size_t m_stack_size = 0;
        std::unordered_map<std::string, Var> m_vars;
};