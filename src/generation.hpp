#pragma once

#include <iostream>

#include <cmath>
#include <cstdint>
#include <sstream>
#include <vector>
#include <cassert>

#include <optional>
#include <unordered_map>

#include <fstream>

#include "tokenization.hpp"
#include "parser.hpp"

class Generator
{
    public:
        inline explicit Generator(NodeProg prog)
            : m_prog(std::move(prog))
        {
        }

        void gen_term(const NodeTerm* term)
        {
            struct TermVisitor
            {
                Generator* gen;

                void operator()(const NodeTermIntLit* term_int_lit) const
                {
                    gen->mov_Var((gen->m_mem_size + 1) * 8, term_int_lit->int_lit.value.value());
                }

                void operator()(const NodeTermIdent* term_ident) const
                {
                    if (!gen->m_vars.contains(term_ident->ident.value.value()))
                    {
                        std::cerr << "Undeclared Identifier : " << term_ident->ident.value.value() << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    const auto& var = gen->m_vars.at(term_ident->ident.value.value());

                    gen->get_Var("rax", (var.mem_loc + 1) * 8);
                    gen->mov_Var((gen->m_mem_size + 1) * 8, "rax");
                }
            };

            TermVisitor visitor { .gen = this };
            std::visit(visitor, term->var);
        }

        void gen_expr(const NodeExpr* expr)
        {
            struct ExprVisitor
            {
                Generator* gen;

                void operator()(const NodeTerm* term) const
                {
                    gen->gen_term(term);
                }

                void operator()(const NodeBinExpr* bin_expr) const
                {
                    gen->gen_expr(bin_expr->var->lhs);
                    gen->gen_expr(bin_expr->var->rhs);

                    gen->use_Var("rdx", (gen->m_mem_size - 1) * 8);
                    gen->use_Var("rax", (gen->m_mem_size + 1) * 8);

                    gen->m_temp << "    add rax, rdx\n";
                    
                    gen->mov_Var((gen->m_mem_size + 1) * 8, "rax");
                }
            };
            
            ExprVisitor visitor { .gen = this };
            std::visit(visitor, expr->var);
        }
        
        void gen_stmt(const NodeStmt* stmt)
        {
            struct StmtVisitor
            {
                Generator* gen;

                void operator()(const NodeStmtDox* stmt_dox) const
                {
                    gen->gen_expr(stmt_dox->expr);

                    gen->m_temp << "\n    ; exit\n";
                    gen->use_Var("rcx", gen->m_mem_size * 8);
                    gen->m_temp << "    call ExitProcess\n";
                    gen->m_temp << "    ; /exit\n\n";
                }

                void operator()(const NodeStmtStreetSign* stmt_StreetSign) const
                {
                    if (gen->m_vars.contains(stmt_StreetSign->ident.value.value()))
                    {
                        std::cerr << "Identifier Is Already Used : " << stmt_StreetSign->ident.value.value() << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    gen->m_vars.insert({ stmt_StreetSign->ident.value.value(), Var { .mem_loc = gen->m_mem_size } });
                    gen->gen_expr(stmt_StreetSign->expr);
                }
            };
            
            StmtVisitor visitor { .gen = this };
            std::visit(visitor, stmt->var);
        }

        [[nodiscard]] std::string gen_prog()
        {
            m_output << "section .text\n";
            m_output << "    global main\n\n";
            
            // m_output << "    extern printf\n";
            m_output << "    extern ExitProcess\n\n";
            
            m_output << "main:\n";
            m_output << "    push rbp\n";
            m_output << "    mov rbp, rsp\n";
            m_output << "    sub rsp, ";

            for (const NodeStmt* stmt : m_prog.stmts)
            {
                gen_stmt(stmt);
            }

            m_output << ceil(static_cast<float>(m_size_counter) / 2) * 16 << "\n\n";

            m_output << m_temp.rdbuf();

            m_output << "    mov rcx, 0\n";
            m_output << "    call ExitProcess";

            return m_output.str();
        }
    
    private:
        void push(const std::string& reg)
        {
            m_temp << "    push " << reg << "\n";
            
            m_stack_size++;
            m_size_counter++;
        }

        void pop(const std::string& reg)
        {
            m_temp << "    pop " << reg << "\n";
            
            m_stack_size--;
        }

        // value can be a register
        void mov_Var(const uint64_t& mem_loc, const std::string& value)
        {
            m_temp << "    mov QWORD [rbp - " << mem_loc << "], " << value << "\n";
            
            m_mem_size++;
            m_size_counter++;
        }

        void use_Var(const std::string& reg, const uint64_t& mem_loc)
        {
            m_temp << "    mov " << reg << ", QWORD [rbp - " << mem_loc << "]" << "\n";
            
            m_mem_size--;
        }

        void get_Var(const std::string& reg, const uint64_t& mem_loc)
        {
            m_temp << "    mov " << reg << ", QWORD [rbp - " << mem_loc << "]" << "\n";
        }

        struct Var
        {
            size_t mem_loc;
        };

        const NodeProg m_prog;
        std::stringstream m_temp;
        std::stringstream m_output;

        size_t m_stack_size = 0;
        size_t m_mem_size = 0;

        size_t m_size_counter = 0;

        std::unordered_map<std::string, Var> m_vars;
};