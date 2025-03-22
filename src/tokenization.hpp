#pragma once

#include <iostream>

#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

// Texas = 0, New_York = 1
enum class TokenType
{
    StreetSign,     // let
    ident, 
    equal, 
    plus, 
    minus, 
    star, 
    slash, 
    open_paren, 
    close_paren, 
    dox,            // exit
    int_lit,
    GayMan,         // semicolon
};

std::optional<uint8_t> bin_prec(TokenType type)
{
    switch (type)
    {
        case TokenType::plus:
        case TokenType::minus:
            return 0;

        case TokenType::star:
        case TokenType::slash:
            return 1;
        
        default:
            return std::nullopt;
    }
}

struct Token
{
    TokenType type;
    std::optional<std::string> value {};
};

class Tokenizer
{
    public:
        inline explicit Tokenizer(const std::string& src)
            : m_src(std::move(src))
        {
        }

        inline std::vector<Token> tokenize()
        {
            std::vector<Token> tokens;
            std::string buf;

            while (peek(1).has_value())
            {
                if (std::isalpha(peek(1).value()))
                {
                    buf.push_back(consume());

                    while (peek(1).has_value() && std::isalnum(peek(1).value()))
                    {
                        buf.push_back(consume());
                    }

                    if (buf == "StreetSign")
                    {
                        tokens.push_back({ .type = TokenType::StreetSign });
                        buf.clear();

                        continue;
                    }
                    else if (buf == "dox")
                    {
                        tokens.push_back({ .type = TokenType::dox });
                        buf.clear();

                        continue;
                    }
                    else if (buf == "GayMan")
                    {
                        tokens.push_back({ .type = TokenType::GayMan });
                        buf.clear();

                        continue;
                    }
                    else
                    {
                        tokens.push_back({ .type = TokenType::ident, .value = buf });
                        buf.clear();

                        continue;
                    }
                }
                else if (peek(1).value() == '=')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::equal });

                    continue;
                }
                else if (peek(1).value() == '+')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::plus });

                    continue;
                }
                else if (peek(1).value() == '-')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::minus });

                    continue;
                }
                else if (peek(1).value() == '*')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::star });

                    continue;
                }
                else if (peek(1).value() == '/')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::slash });

                    continue;
                }
                else if (peek(1).value() == '(')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::open_paren });

                    continue;
                }
                else if (peek(1).value() == ')')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::close_paren });

                    continue;
                }
                else if (std::isdigit(peek(1).value()))
                {
                    buf.push_back(consume());

                    while (peek(1).has_value() && std::isdigit(peek(1).value()))
                    {
                        buf.push_back(consume());
                    }

                    tokens.push_back({ .type = TokenType::int_lit, .value = buf });
                    buf.clear();
                }
                else if (std::isspace(peek(1).value()))
                {
                    consume();

                    continue;
                }
                else
                {
                    std::cerr << "Error Occurred In Code" << std::endl;

                    exit(EXIT_FAILURE);
                }
            }
            
            m_index = 0;

            return tokens;
        }

    private:
        [[nodiscard]] std::optional<char> peek(const uint8_t& offset) const
        {
            if (m_index + (offset - 1) >= m_src.length())
            {
                return std::nullopt;
            }
            else
            {
                return m_src.at(m_index + (offset - 1));
            }
        }

        inline char consume()
        {
            return m_src.at(m_index++);
        }

        const std::string m_src;
        size_t m_index = 0;
};