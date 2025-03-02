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
        inline explicit Generator(NodeDox root) : m_root(std::move(root)) {}

        [[nodiscard]] std::string generate() const
        {
            std::stringstream output;
            
            output << "section .text\n";
            output << "    global main\n\n";
            
            output << "    extern printf\n";
            output << "    extern ExitProcess\n\n";
            
            output << "main:\n";
            output << "    sub rsp, 40\n\n";

            output << "    mov rcx, " << m_root.expr.int_lit.value.value() << "\n";
            output << "    call ExitProcess";

            return output.str();
        }
    
    private:
        const NodeDox m_root;
};