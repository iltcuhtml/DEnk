#pragma once

#include <iostream>

#include <sstream>
#include <vector>

#include <optional>

#include <fstream>

enum class TokenType
{
    Bestimme,       // let (int64_t)
    falls,          // if
    sonst,          // else
    dann,           // then
    Beende,         // exit
    ident,          // [identifier]
    int_lit,        // [int literal]
    dot,            // . (;)
    als,            // = (as)
    gleich,         // ==
    ungleich,       // !=
    größer,         // >
    kleiner,        // <
    und,            // &&
    oder,           // ||
    nicht,          // ! (not)
    plus,           // +
    minus,          // -
    star,           // *
    slash,          // /
    open_paren,     // (
    close_paren,    // )
    open_curly,     // {
    close_curly,    // }
};

inline std::optional<size_t>bin_prec(const TokenType type)
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
        explicit Tokenizer(const std::string& src)
            : m_src(std::move(src))
        {
        }

        std::vector<Token> tokenize()
        {
            std::vector<Token> tokens;
            std::string buf;

            while (peek(1).has_value())
            {
                char32_t ch = peek(1).value();

                if (is_alpha(ch))
                {
                    buf += consume();

                    while (peek(1).has_value() && is_alnum(peek(1).value()))
                    {
                        buf += consume();
                    }

                    if (buf == "Bestimme")
                    {
                        tokens.push_back({ .type = TokenType::Bestimme });
                    }
                    else if (buf == "falls")
                    {
                        tokens.push_back({ .type = TokenType::falls });
                    }
                    else if (buf == "sonst")
                    {
                        tokens.push_back({ .type = TokenType::sonst });
                    }
                    else if (buf == "dann")
                    {
                        tokens.push_back({ .type = TokenType::dann });
                    }
                    else if (buf == "Beende")
                    {
                        tokens.push_back({ .type = TokenType::Beende });
                    }
                    else if (buf == "als")
                    {
                        tokens.push_back({ .type = TokenType::als });
                    }
                    else if (buf == "gleich")
                    {
                        tokens.push_back({ .type = TokenType::gleich });
                    }
                    else if (buf == "ungleich")
                    {
                        tokens.push_back({ .type = TokenType::ungleich });
                    }
                    else if (buf == "größer")
                    {
                        tokens.push_back({ .type = TokenType::größer });
                    }
                    else if (buf == "kleiner")
                    {
                        tokens.push_back({ .type = TokenType::kleiner });
                    }
                    else if (buf == "und")
                    {
                        tokens.push_back({ .type = TokenType::und });
                    }
                    else if (buf == "oder")
                    {
                        tokens.push_back({ .type = TokenType::oder });
                    }
                    else if (buf == "nicht")
                    {
                        tokens.push_back({ .type = TokenType::nicht });
                    }
                    else
                    {
                        tokens.push_back({ .type = TokenType::ident, .value = buf });
                    }

                    buf.clear();
                }
                else if (ch == U'.')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::dot });
                }
                else if (ch == U'+')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::plus });
                }
                else if (ch == U'-')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::minus });
                }
                else if (ch == U'*')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::star });
                }
                else if (ch == U'/')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::slash });
                }
                else if (ch == U'(')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::open_paren });
                }
                else if (ch == U')')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::close_paren });
                }
                else if (ch == U'{')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::open_curly });
                }
                else if (ch == U'}')
                {
                    consume();
                    tokens.push_back({ .type = TokenType::close_curly });
                }
                else if (is_digit(ch))
                {
                    buf += consume();

                    while (peek(1).has_value() && std::isdigit(peek(1).value()))
                    {
                        buf += consume();
                    }

                    tokens.push_back({ .type = TokenType::int_lit, .value = buf });
                    buf.clear();
                }
                else if (std::isspace(ch))
                {
                    consume();
                }
                else
                {
                    std::cerr << "Fehler: Ein Syntaxfehler ist aufgetreten" << std::endl;

                    exit(EXIT_FAILURE);
                }
            }
            
            m_index = 0;

            return tokens;
        }

    private:
        [[nodiscard]] std::optional<char32_t> peek(const size_t& offset) const
        {
            size_t peek_index = m_index + (offset - 1);

            if (peek_index >= m_src.size())
                return std::nullopt;

            size_t first = m_src.at(peek_index);

            if (first < 0x80)
                return first;

            if ((first >> 5) == 0x06 && peek_index + 1 < m_src.size())
                return ((first & 0x1F) << 6) | (m_src.at(peek_index + 1) & 0x3F);

            if ((first >> 4) == 0x0E && peek_index + 2 < m_src.size())
                return ((first & 0x0F) << 12) | ((m_src.at(peek_index + 1) & 0x3F) << 6)
                        | (m_src.at(peek_index + 2) & 0x3F);

            if ((first >> 3) == 0x1E && peek_index + 3 < m_src.size())
                return ((first & 0x07) << 18) | ((m_src.at(peek_index + 1) & 0x3F) << 12)
                        | ((m_src.at(peek_index + 2) & 0x3F) << 6)
                        | (m_src.at(peek_index + 3) & 0x3F);

            return std::nullopt;
        }

        
        std::string consume()
        {
            if (m_index >= m_src.size())
                return "";

            size_t first = m_src.at(m_index);
            size_t len = 1;

            if (first < 0x80)
                len = 1;
            else if ((first >> 5) == 0x6)
                len = 2;
            else if ((first >> 4) == 0xE)
                len = 3;
            else if ((first >> 3) == 0x1E)
                len = 4;
            else
                len = 1;

            std::string result = m_src.substr(m_index, len);
            
            m_index += len;
            
            return result;
        }

        inline bool is_alpha(char32_t c) const
        {
            return (c >= U'a' && c <= U'z') || (c >= U'A' && c <= U'Z') ||
                   (c >= 0x00C0 && c <= 0x00FF);
        }
    
        inline bool is_alnum(char32_t c) const
        {
            return is_alpha(c) || is_digit(c);
        }
    
        inline bool is_digit(char32_t c) const
        {
            return (c >= U'0' && c <= U'9');
        }
    
        inline bool is_space(char32_t c) const
        {
            return (c == U' ' || c == U'\n' || c == U'\r' || c == U'\t');
        }

        const std::string m_src;
        size_t m_index = 0;
};