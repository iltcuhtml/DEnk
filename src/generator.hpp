#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <optional>
#include <fstream>
#include <ranges>
#include <cmath>
#include <cassert>

#include "tokenizer.hpp"
#include "parser.hpp"

class Generator
{
    public:
        inline explicit Generator(NodeProg prog)
            : m_prog(std::move(prog))
            {}

        std::string gen_prog()
        {
            m_output << "section .text\n";
            m_output << "    global _start\n\n";        
            
            for (const NodeStmt* stmt : m_prog.stmts)
                gen_stmt(stmt);
            
            m_output << "\n_start:\n";
            m_output << "    push rbp\n";
            m_output << "    mov rbp, rsp\n";
            m_output << "    sub rsp, "
                     << align_stack(m_max_stack_size + m_max_mem_size)
                     << "\n\n";
            
            m_output << m_temp.rdbuf();

            return m_output.str();
        }
    
    private:
        /* Internal State */
        struct Var
        {
            std::string name;
            size_t mem_loc;
        };

        const NodeProg m_prog;
        std::stringstream m_temp, m_output;
        std::vector<std::string> m_extern;

        std::vector<Var> m_vars;
        std::vector<size_t> m_scopes;

        size_t m_mem_size = 0;
        size_t m_stack_size = 0;
        size_t m_max_mem_size = 0;
        size_t m_max_stack_size = 0;

        size_t m_label_count = 0;

        /* Expression Generation */
        void gen_expr(const NodeExpr* expr)
        {
            struct Visitor
            {
                Generator& gen;

                void operator()(const NodeTerm* term) const { gen.gen_term(term); }

                void operator()(const NodeBinExpr* bin_expr) const { gen.gen_bin_expr(bin_expr); }
            };

            std::visit(Visitor{ *this }, expr->var);
        }

        void gen_term(const NodeTerm* term)
        {
            struct Visitor
            {
                Generator& gen;

                void operator()(const NodeTermIntLit* t) const
                {
                    gen.store_value(gen.temp_mem_loc(), t->int_lit.value.value());
                }

                void operator()(const NodeTermIdent* t) const
                {
                    auto it = 
                        std::ranges::find_if(
                            gen.m_vars, 
                            [&](const Var& var)
                            {
                                return var.name == t->ident.value.value();
                            }
                        );
                    
                    if (it == gen.m_vars.cend())
                    {
                        std::cerr << "Fehler: Bezeichner '" << t->ident.value.value() << "' ist nicht deklariert\n";

                        exit(EXIT_FAILURE);
                    }

                    gen.load_var("rax", (it->mem_loc + 1) * 8);
                    gen.store_value(gen.temp_mem_loc(), "rax");
                }

                void operator()(const NodeTermParen* t) const
                {
                    gen.gen_expr(t->expr);
                }
            };

            std::visit(Visitor{ *this }, term->var);
        }

        void gen_bin_expr(const NodeBinExpr* bin_expr)
        {
            auto& gen = *this;
        
            auto binary_op = [&](const NodeExpr* lhs, const NodeExpr* rhs, const std::string& instr)
            {
                gen.gen_expr(rhs);
                gen.gen_expr(lhs);
            
                gen.consume_var("rax", gen.temp_mem_loc(true));
            
                if (instr == "div")
                {
                    gen.m_temp << "    cqo\n";
                    gen.m_temp << "    idiv QWORD [rbp - " << gen.temp_mem_loc(true) << "]\n";
                }
                else
                {
                    gen.m_temp << "    " << instr << " rax, QWORD [rbp - " << gen.temp_mem_loc(true) << "]\n";
                }
            
                gen.overwrite_value(gen.temp_mem_loc(true), "rax");
            };
        
            switch (bin_expr->op)
            {
                case BinOp::Add:
                    binary_op(bin_expr->lhs, bin_expr->rhs, "add");

                    break;

                case BinOp::Sub:
                    binary_op(bin_expr->lhs, bin_expr->rhs, "sub");
                    
                    break;

                case BinOp::Mul:
                    binary_op(bin_expr->lhs, bin_expr->rhs, "imul");
                    
                    break;

                case BinOp::Div:
                    binary_op(bin_expr->lhs, bin_expr->rhs, "div");
                    
                    break;
                    
                default:
                    // Handle error or unsupported operation
                    break;
            }
        }
        
        /* Statement Generation */
        void gen_stmt(const NodeStmt* stmt)
        {
            struct Visitor
            {
                Generator& gen;

                void operator()(const NodeStmtBeende* s) const
                {
                    gen.declare_extern_once("ExitProcess");

                    gen.gen_expr(s->expr);
                    gen.consume_var("rcx", gen.temp_mem_loc());

                    gen.m_temp << "    call ExitProcess\n";
                }

                void operator()(const NodeStmtBestimme* s) const
                {
                    if (gen.find_var(s->ident.value.value()))
                    {
                        std::cerr << "Fehler: Bezeichner '" << s->ident.value.value() << "' wird bereits verwendet\n";
                        
                        exit(EXIT_FAILURE);
                    }
                    
                    gen.m_vars.push_back({s->ident.value.value(), gen.m_mem_size});
                    gen.gen_expr(s->expr);
                }

                void operator()(const NodeScope* scope) const
                {
                    gen.gen_scope(scope);
                }

                void operator()(const NodeStmtFalls* s) const
                {
                    gen.gen_expr(s->expr);
                    gen.consume_var("rax", gen.temp_mem_loc());
                    
                    const std::string label = gen.create_label();

                    gen.m_temp << "    test rax, rax\n";
                    gen.m_temp << "    jz " << label << "\n";
                    
                    gen.gen_scope(s->scope);
                    gen.m_temp << label << ":\n";
                }
            };
            
            std::visit(Visitor{ *this }, stmt->var);
        }

        void gen_scope(const NodeScope* scope)
        {
            begin_scope();

            for (const NodeStmt* stmt : scope->stmts)
                gen_stmt(stmt);

            end_scope();
        }

        /* Assembly Helpers */
        void store_value(size_t mem, const std::string& val)
        {
            m_temp << "    mov QWORD [rbp - " << mem << "], " << val << "\n";
            
            track_mem();
        }

        void overwrite_value(size_t mem, const std::string& val)
        {
            m_temp << "    mov QWORD [rbp - " << mem << "], " << val << "\n";
        }

        void load_var(const std::string& reg, size_t mem)
        {
            m_temp << "    mov " << reg << ", QWORD [rbp - " << mem << "]\n";
        }

        void consume_var(const std::string& reg, size_t mem)
        {
            load_var(reg, mem);

            m_mem_size--;
        }

        void declare_extern_once(const std::string& name)
        {
            if (std::ranges::find(m_extern, name) == m_extern.end())
            {
                m_output << "    extern " << name << "\n";

                m_extern.push_back(name);
            }
        }

        /* Scope Helpers */
        void begin_scope()
        {
            m_scopes.push_back(m_vars.size());
        }

        void end_scope()
        {
            const size_t pop_count = m_vars.size() - m_scopes.back();

            m_mem_size -= pop_count;

            m_vars.resize(m_scopes.back());
            m_scopes.pop_back();
        }

        std::optional<Var> find_var(const std::string& name)
        {
            for (const auto& v : m_vars)
                if (v.name == name)
                    return v;

            return std::nullopt;
        }

        size_t temp_mem_loc(bool offset = false) const
        {
            return (m_mem_size + (offset ? 1 : 0)) * 8;
        }

        void track_mem()
        {
            m_mem_size++;

            m_max_mem_size = std::max(m_max_mem_size, m_mem_size);
        }

        size_t align_stack(size_t raw) const
        {
            return static_cast<size_t>(std::ceil(raw / 2.0)) * 16;
        }

        std::string create_label()
        {
            return ".L" + std::to_string(m_label_count++);
        }
};