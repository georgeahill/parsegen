#pragma once

#include <string>

typedef enum
{
    TOKEN_TOKEN
} TokenType;

struct Token
{
    Token(std::string lexeme) : lexeme(lexeme){};
    Token() : lexeme(""){};
    TokenType type;
    std::string lexeme;
    int posX = 0;
    int posY = 0;

    std::string posStr()
    {
        std::string out = "(";
        out += std::to_string(posY);
        out += ", ";
        out += std::to_string(posX);
        out += ")";
        return out;
    }
};