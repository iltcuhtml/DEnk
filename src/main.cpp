#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

/* make all
 * ./exe/jschlang ./io/test.jschlang
 */

enum class TokenType
{
    dox,
    int_lit,
    GayMan
};

struct Token
{
    TokenType type;
    std::optional<std::string> value{};
};

std::vector<Token> tokenize(const std::string& str)
{
    std::vector<Token> tokens{};
    std::string buf;
    
    for (uint64_t i = 0; i < str.length(); i++)
    {
        char c = str.at(i);

        if (std::isalpha(c))
        {
            buf.push_back(c);
            i++;

            while (i < str.length() && std::isalnum(str.at(i)))
            {
                buf.push_back(str.at(i));
                i++;
            }

            i--;

            if (buf == "dox")
            {
                tokens.push_back({.type = TokenType::dox});
                buf.clear();
                continue;
            } else if (buf == "GayMan")
            {
                tokens.push_back({.type = TokenType::GayMan});
                buf.clear();
                continue;
            } else
            {
                std::cerr << "Error Occurred In Token" << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c))
        {
            buf.push_back(c);
            i++;

            while (std::isdigit(str.at(i)))
            {
                buf.push_back(str.at(i));
                i++;
            }
            
            i--;
            tokens.push_back({.type = TokenType::int_lit, .value = buf});
            buf.clear();
        } else if (std::isspace(c))
        {
            continue;
        } else
        {
            std::cerr << "Error Occurred In Code" << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens)
{
    std::stringstream output;

    output << "section .text\n";
    output << "    global main\n\n";

    output << "    extern printf\n";
    output << "    extern ExitProcess\n\n";

    output << "main:\n";
    output << "    sub rsp, 40\n\n";

    for (uint64_t i = 0; i < tokens.size(); i++)
    {
        const Token& token = tokens.at(i);

        if (token.type == TokenType::dox)
        {
            if (i + 1 < tokens.size() && tokens.at(i + 1).type == TokenType::int_lit)
            {
                if (i + 2 < tokens.size() && tokens.at(i + 2).type == TokenType::GayMan)
                {
                    output << "    mov rcx, " << tokens.at(i + 1).value.value() << "\n";
                    output << "    call ExitProcess";
                }
            }
        }
    }
    
    return output.str();
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Add Jschlang File (*.jschlang)" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;

    {
        std::stringstream contents_stream;

        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents);
    // std::cout << tokens_to_asm(tokens);

    {
        system("mkdir out");

        std::fstream file("out/out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }

    system("nasm -f win64 out/out.asm -o out/out.o");
    system("gcc out/out.o -o out/out");

    return EXIT_SUCCESS;
}