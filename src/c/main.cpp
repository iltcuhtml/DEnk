#include <iostream>

#include <cstdint>
#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

/* make all
 * ./exe/hl ./Hanlang/test.hl
 */

bool isHangul(wchar_t c)
{
    return (c >= 0xAC00 && c <= 0xD7A3);
}

enum class TokenType
{
    exit,
    int_lit,
    semicolon
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

        if (isHangul(c) || std::isalpha(c))
        {
            buf.push_back(c);
            i++;

            while (std::isalnum(str.at(i)))
            {
                buf.push_back(str.at(i));
                i++;
            }

            i--;

            if (buf == "종료")
            {
                tokens.push_back({.type = TokenType::exit});
                buf.clear();
                continue;
            } else
            {
                std::cerr << "종료 오류" << std::endl;
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
        } else if (c == ';')
        {
            tokens.push_back({.type = TokenType::semicolon});
        } else if (std::isspace(c))
        {
            continue;
        } else
        {
            std::cerr << "코드 오류" << std::endl;
        }
    }

    return tokens;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
        std::cerr << "한랭 파일(*.hl)을 추가하세요." << std::endl;

    std::string contents;

    {
        std::stringstream contents_stream;

        std::fstream input(argv[1], std::ios::in);
        contents_stream << input.rdbuf();
        
        contents = contents_stream.str();
    }

    tokenize(contents);

    return EXIT_SUCCESS;
}