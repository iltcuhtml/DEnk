#pragma once

#include <iostream>

#include <cmath>
#include <cstdint>
#include <sstream>
#include <vector>
#include <cassert>

#include <optional>

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
                    auto it = 
                        std::find_if(
                            gen->m_vars.cbegin(), 
                            gen->m_vars.cend(), 
                            [&](const Var& var)
                            {
                                return var.name == term_ident->ident.value.value();
                            }
                        );
                    
                    if (it == gen->m_vars.cend())
                    {
                        std::cerr << "Fehler: Bezeichner '" << term_ident->ident.value.value() << "' ist nicht deklariert" << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    gen->get_Var("rax", ((*it).mem_loc + 1) * 8);
                    gen->mov_Var((gen->m_mem_size + 1) * 8, "rax");
                }

                void operator()(const NodeTermParen* term_paren) const
                {
                    gen->gen_expr(term_paren->expr);
                }
            };

            TermVisitor visitor { .gen = this };
            std::visit(visitor, term->var);
        }

        void gen_bin_expr(const NodeBinExpr* bin_expr)
        {
            struct BinExprVisitor
            {
                Generator* gen;

                void operator()(const NodeBinExprAdd* add) const
                {
                    gen->m_temp << "\n    ; add\n";

                    gen->gen_expr(add->rhs);
                    gen->gen_expr(add->lhs);

                    gen->consume_Var("rax", gen->m_mem_size * 8);
                    gen->m_temp << "    add rax, QWORD [rbp - " << gen->m_mem_size * 8 << "]\n";
                    gen->overwrite_Var(gen->m_mem_size * 8, "rax");

                    gen->m_temp << "    ; /add\n\n";
                }

                void operator()(const NodeBinExprSub* sub) const
                {
                    gen->m_temp << "\n    ; sub\n";

                    gen->gen_expr(sub->rhs);
                    gen->gen_expr(sub->lhs);

                    gen->consume_Var("rax", gen->m_mem_size * 8);
                    gen->m_temp << "    sub rax, QWORD [rbp - " << gen->m_mem_size * 8 << "]\n";
                    gen->overwrite_Var(gen->m_mem_size * 8, "rax");

                    gen->m_temp << "    ; /sub\n\n";
                }

                void operator()(const NodeBinExprMul* mul) const
                {
                    gen->m_temp << "\n    ; mul\n";

                    gen->gen_expr(mul->rhs);
                    gen->gen_expr(mul->lhs);

                    gen->consume_Var("rax", gen->m_mem_size * 8);
                    gen->m_temp << "    imul QWORD [rbp - " << gen->m_mem_size * 8 << "]\n";
                    gen->overwrite_Var(gen->m_mem_size * 8, "rax");

                    gen->m_temp << "    ; /mul\n\n";
                }

                void operator()(const NodeBinExprDiv* div) const
                {
                    gen->m_temp << "\n    ; div\n";

                    gen->gen_expr(div->rhs);
                    gen->gen_expr(div->lhs);

                    gen->consume_Var("rax", gen->m_mem_size * 8);
                    gen->m_temp << "    cqo\n";
                    gen->m_temp << "    idiv QWORD [rbp - " << gen->m_mem_size * 8 << "]\n";
                    gen->overwrite_Var(gen->m_mem_size * 8, "rax");

                    gen->m_temp << "    ; /div\n\n";
                }
            };

            BinExprVisitor visitor { .gen = this };
            std::visit(visitor, bin_expr->var);
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
                    gen->gen_bin_expr(bin_expr);
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

                void operator()(const NodeStmtBeende* stmt_beende) const
                {
                    gen->m_temp << "\n    ; exit\n";
                    
                    gen->gen_expr(stmt_beende->expr);

                    gen->consume_Var("rcx", gen->m_mem_size * 8);
                    gen->m_temp << "    call ExitProcess\n";

                    gen->m_temp << "    ; /exit\n\n";
                }

                void operator()(const NodeStmtBestimme* stmt_bestimme) const
                {
                    auto it = 
                        std::find_if(
                            gen->m_vars.cbegin(), 
                            gen->m_vars.cend(), 
                            [&](const Var& var)
                            {
                                return var.name == stmt_bestimme->ident.value.value();
                            }
                        );
                    
                    if (it != gen->m_vars.cend())
                    {
                        std::cerr << "Fehler: Bezeichner '" << stmt_bestimme->ident.value.value() << "' wird bereits verwendet" << std::endl;

                        exit(EXIT_FAILURE);
                    }

                    gen->m_vars.push_back(
                        Var
                        {
                            .name = stmt_bestimme->ident.value.value(), 
                            .mem_loc = gen->m_mem_size
                        }
                    );

                    gen->gen_expr(stmt_bestimme->expr);
                }

                void operator()(const NodeScope* scope) const
                {
                    gen->begin_scope();

                    for (const NodeStmt* stmt : scope->stmts)
                    {
                        gen->gen_stmt(stmt);
                    }

                    gen->end_scope();
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

            m_output << ceil(static_cast<float>(m_max_stack_size + m_max_mem_size) / 2) * 16 << "\n\n";

            m_output << m_temp.rdbuf();

            m_output << "\n    mov rcx, 0\n";
            m_output << "    call ExitProcess";

            return m_output.str();
        }
    
    private:
        void push(const std::string& reg)
        {
            m_temp << "    push " << reg << "\n";
            
            m_stack_size++;

            if (m_max_stack_size < m_stack_size)
            {
                m_max_stack_size = m_stack_size;
            }
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

            if (m_max_mem_size < m_mem_size)
            {
                m_max_mem_size = m_mem_size;
            }
        }

        // value can be a register
        void overwrite_Var(const uint64_t& mem_loc, const std::string& value)
        {
            m_temp << "    mov QWORD [rbp - " << mem_loc << "], " << value << "\n";
        }

        void get_Var(const std::string& reg, const uint64_t& mem_loc)
        {
            m_temp << "    mov " << reg << ", QWORD [rbp - " << mem_loc << "]" << "\n";
        }

        void consume_Var(const std::string& reg, const uint64_t& mem_loc)
        {
            m_temp << "    mov " << reg << ", QWORD [rbp - " << mem_loc << "]" << "\n";

            m_mem_size--;
        }

        void begin_scope()
        {
            m_scopes.push_back(m_vars.size());
        }

        void end_scope()
        {
            size_t pop_count = m_vars.size() - m_scopes.back();

            m_mem_size -= pop_count;    // This CAN Make A Problem MAYBE

            for (size_t i = 0; i < pop_count; i++)
            {
                m_vars.pop_back();
            }

            m_scopes.pop_back();
        }

        struct Var
        {
            std::string name;

            size_t mem_loc;
        };

        const NodeProg m_prog;
        std::stringstream m_temp;
        std::stringstream m_output;

        size_t m_stack_size = 0;
        size_t m_mem_size = 0;

        size_t m_max_stack_size = 0;
        size_t m_max_mem_size = 0;

        std::vector<Var> m_vars;
        std::vector<size_t> m_scopes;
};