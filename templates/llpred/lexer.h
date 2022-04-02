#pragma once

#include "token.h"

class Lexer
{
public:
    Lexer(std::string input) : text(input){};

    Token *peek()
    {
        if (ptr >= text.size())
            return new Token("$"); // end of string for parsertable

        Token *newToken = new Token();

        int tmpPtr = ptr;

        while (text[tmpPtr] != ' ' && tmpPtr < text.size())
        {
            newToken->lexeme.push_back(text[tmpPtr]);
            ++tmpPtr;
        }

        return newToken;
    }

    Token *last()
    {
        return this->lastToken;
    }

    Token *next()
    {
        this->lastToken = this->current;
        if (ptr >= text.size())
        {
            auto tok = new Token("$");
            tok->posY = linePtr;
            tok->posX = thisLinePtr;
            current = tok;
            return tok;
        }; // end of string for parsertable

        Token *newToken = new Token();

        while (text[ptr] != ' ' && ptr < text.size())
        {
            newToken->lexeme.push_back(text[ptr]);
            ++ptr;
            ++thisLinePtr;
            if (text[ptr] == '\n')
            {
                linePtr += 1;
                thisLinePtr = 0;
            }
        }
        ++ptr; // skip space
        ++thisLinePtr;
        if (text[ptr] == '\n')
        {
            linePtr += 1;
            thisLinePtr = 0;
        }

        newToken->posY = linePtr;
        newToken->posX = thisLinePtr - newToken->lexeme.size();
        current = newToken;
        return newToken;
    }

private:
    std::string text;
    int ptr = 0;
    int linePtr = 1;
    int thisLinePtr = 0;

    Token *current;
    Token *lastToken;
};
