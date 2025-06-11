#pragma once

#include <iostream>
#include <sstream>
#include <vector>
#include <optional>
#include <fstream>
#include <unordered_map>

enum class TokenType
{
    ident, int_lit, 
    dot, plus, minus, star, slash, 
    open_paren, close_paren, open_curly, close_curly, 
    Bestimme, als, 
    Ändere, zu, 
    Falls, Sonst, dann, gleich, ungleich, kleiner, größer, und, oder, nicht, 
    Beende, mit, 
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
        explicit Tokenizer(const std::string& src) : m_src(src) {}

        std::vector<Token> tokenize()
        {
            static const std::unordered_map<std::string, TokenType> keywords = {
                {"Bestimme", TokenType::Bestimme}, {"bestimme", TokenType::Bestimme}, 
                {"als", TokenType::als}, {"Als", TokenType::als}, 
                
                {"Ändere", TokenType::Ändere}, {"ändere", TokenType::Ändere}, 
                {"zu", TokenType::zu}, {"Zu", TokenType::zu}, 
                
                {"Falls", TokenType::Falls}, {"falls", TokenType::Falls}, 
                {"Sonst", TokenType::Sonst}, {"sonst", TokenType::Sonst}, 
                {"dann", TokenType::dann}, {"Dann", TokenType::dann}, 

                {"gleich", TokenType::gleich}, {"Gleich", TokenType::gleich}, 
                {"ungleich", TokenType::ungleich}, {"Ungleich", TokenType::ungleich}, 
                {"kleiner", TokenType::kleiner}, {"Kleiner", TokenType::kleiner}, 
                {"größer", TokenType::größer}, {"Größer", TokenType::größer}, 
                {"und", TokenType::und}, {"Und", TokenType::und}, 
                {"oder", TokenType::oder}, {"Oder", TokenType::oder}, 
                {"nicht", TokenType::nicht}, {"Nicht", TokenType::nicht}, 

                {"Beende", TokenType::Beende}, {"beende", TokenType::Beende}, 
                {"mit", TokenType::mit}, {"Mit", TokenType::mit}, 
            };

            std::vector<Token> tokens;

            while (peek().has_value())
            {
                char32_t ch = peek().value();

                if (is_alpha(ch))
                {
                    std::string ident = consume_while([&](char32_t c) { return is_alnum(c); });

                    auto it = keywords.find(ident);

                    if (it != keywords.end())
                        tokens.push_back({ .type = it->second });
                    else
                        tokens.push_back({ .type = TokenType::ident, .value = ident });
                }
                else if (is_digit(ch))
                {
                    std::string number = consume_while([&](char32_t c) { return is_digit(c); });
                    
                    tokens.push_back({ .type = TokenType::int_lit, .value = number });
                }
                else if (is_space(ch))
                {
                    consume();
                }
                else
                {
                    const auto one_char = consume();

                    if (one_char == ".")
                        tokens.push_back({ .type = TokenType::dot });
                    
                    else if (one_char == "+")
                        tokens.push_back({ .type = TokenType::plus });
                    
                    else if (one_char == "-")
                        tokens.push_back({ .type = TokenType::minus });
                    
                    else if (one_char == "*")
                        tokens.push_back({ .type = TokenType::star });
                    
                    else if (one_char == "/")
                    {
                        if (peek().has_value())
                        {
                            if (peek().value() == U'/')
                                consume_while([&](char32_t c) { return c != U'\n'; });

                            else if (peek().value() == U'*')
                            {
                                consume(); // consume '/'
                                consume(); // consume '*'

                                while (true)
                                {
                                    consume_while([&](char32_t c) { return c != U'*'; });
                                
                                    if (!peek().has_value())
                                    {
                                        std::cerr << "Fehler: Mehrzeiliger Kommentar wurde nicht korrekt geschlossen (erwartetes '*/')\n";

                                        exit(EXIT_FAILURE);
                                    }
                                
                                    if (peek().value() == U'*')
                                    {
                                        consume(); // consume '*'
                                    
                                        if (peek().has_value() && peek().value() == U'/')
                                        {
                                            consume(); // consume '/'

                                            break;
                                        }
                                    }
                                }
                            }
                        }
                        else
                            tokens.push_back({ .type = TokenType::slash });
                    }
                    else if (one_char == "(")
                        tokens.push_back({ .type = TokenType::open_paren });
                    
                    else if (one_char == ")")
                        tokens.push_back({ .type = TokenType::close_paren });
                    
                    else if (one_char == "{")
                        tokens.push_back({ .type = TokenType::open_curly });
                    
                    else if (one_char == "}")
                        tokens.push_back({ .type = TokenType::close_curly });
                    
                    else
                    {
                        std::cerr << "Fehler: Ein Syntaxfehler ist aufgetreten\n";

                        exit(EXIT_FAILURE);
                    }
                }
            }

            m_index = 0;

            return tokens;
        }

    private:
        template <typename Predicate>
        std::string consume_while(Predicate pred)
        {
            std::string result;

            while (peek().has_value() && pred(peek().value()))
                result += consume();
            
            return result;
        }

        std::optional<char32_t> peek(const size_t& offset = 0) const
        {
            size_t peek_index = m_index + offset;
            
            if (peek_index >= m_src.size())
                return std::nullopt;

            unsigned char first = m_src[peek_index];

            if (first < 0x80)
                return first;

            if ((first >> 5) == 0x06 && peek_index + 1 < m_src.size())
                return ((first & 0x1F) << 6) | (m_src[peek_index + 1] & 0x3F);

            if ((first >> 4) == 0x0E && peek_index + 2 < m_src.size())
                return ((first & 0x0F) << 12) | ((m_src[peek_index + 1] & 0x3F) << 6)
                     | (m_src[peek_index + 2] & 0x3F);

            if ((first >> 3) == 0x1E && peek_index + 3 < m_src.size())
                return ((first & 0x07) << 18) | ((m_src[peek_index + 1] & 0x3F) << 12)
                     | ((m_src[peek_index + 2] & 0x3F) << 6)
                     | (m_src[peek_index + 3] & 0x3F);

            return std::nullopt;
        }

        std::string consume()
        {
            if (m_index >= m_src.size())
                return "";

            unsigned char first = m_src[m_index];
            size_t len = 1;

            if (first < 0x80)
                len = 1;
            
            else if ((first >> 5) == 0x06)
                len = 2;
            
            else if ((first >> 4) == 0x0E)
                len = 3;
            
            else if ((first >> 3) == 0x1E)
                len = 4;

            std::string result = m_src.substr(m_index, len);

            m_index += len;

            return result;
        }

        inline bool is_alpha(char32_t c) const
        {
            return (c >= U'a' && c <= U'z') || 
                   (c >= U'A' && c <= U'Z') ||
                   (c >= 0x00C0 && c <= 0x00FF) || // Latin-1 (includes ß, ä, ö, ü, é, etc.)
                   (c >= 0x0100 && c <= 0x017F) || // Latin Extended-A
                   (c >= 0x0180 && c <= 0x024F);   // Latin Extended-B
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