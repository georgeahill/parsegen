#include <stack>
#include <set>
#include <iostream>

#include "parsetree.h"
#include "symbol.h"
#include "parsertable.h"
#include "lexer.h"
#include "token.h"

Util::ParseTree *parse(std::string input, ParserTable *parseTable)
{
    Lexer lexer(input);

    /*

    // input buffer contains w$
    // stack contains S$ where S is start symbol of grammar G

    let a be the first symbol of w
    let X be the top stack symbol (called S for now)

    while (X != $) // while stack not empty
        if ( X = a ) pop the stack and let a be the next symbol of w
        else if ( X is a terminal ) error()
        else if ( M[X,a] is an error entry ) error()
        else if ( M[X,a] = X->Y1Y2...Yk ) {
            output production X->Y1Y2...Yk
            pop the stack
            push Yk,Yk-1,...Y1 onto the stack, with Y1 on top
        }
        let X be the top stack symbol

    */

    Util::ParseTree *tree = new Util::ParseTree(Util::Symbol(parseTable->start.name, Util::SymbolType::NON_TERMINAL));

    std::stack<std::pair<Util::Symbol *, Util::Node *>> parseStack;
    parseStack.push(std::make_pair(new Util::Symbol("$", Util::SymbolType::END), nullptr));
    parseStack.push(std::make_pair(new Util::Symbol(parseTable->start.name, Util::SymbolType::NON_TERMINAL), tree));

    Util::Symbol *X = parseStack.top().first;
    Util::Node *currentPNode = parseStack.top().second;

    Token *a = lexer.next();

    // // special case for initial
    // if (this->parseTable->isSynch(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL)))
    // {
    //     // error - synch (so skip top nonterminal)

    //     parseStack.pop();
    //     std::cout << a->posStr() << ": synch " << a->lexeme << std::endl;
    // }

    while (!X->isEnd())
    {
        if (X->name == a->lexeme && X->isTerminal())
        {
            parseStack.pop();
            a = lexer.next();
        }
        else if (X->isEmpty()) // empty production
        {
            parseStack.pop();
        }
        else if (X->isTerminal())
        {
            // error - no match of non-terminal

            // take synchset to be all other tokens
            // pop, message, continue
            parseStack.pop();
            std::cout << a->posStr() << ": expected " << X->name << std::endl;
        }
        else if ((parseTable->isSynch(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL)) && a->lexeme != "$") || (parseTable->isSynch(*X, Util::Symbol(a->lexeme, Util::SymbolType::END)) && a->lexeme == "$"))
        {
            // error - synch (so skip top nonterminal)

            parseStack.pop();
            std::cout << a->posStr() << ": synch " << a->lexeme << std::endl;
        }
        else if ((parseTable->isError(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL)) && a->lexeme != "$") || (parseTable->isError(*X, Util::Symbol(a->lexeme, Util::SymbolType::END)) && a->lexeme == "$"))
        {
            // error - no parse table entry (so skip input)
            // we could be at the end here!
            if (a->lexeme == "$")
            {
                std::cout << a->posStr() << ": Unexpected end of input" << std::endl;
                break;
            }

            std::cout << a->posStr() << " Unexpected " << a->lexeme;
            auto possibleVals = parseTable->first[std::vector<Util::Symbol>({*X})];
            std::cout << ", expected ";

            for (auto it = possibleVals.begin(); it != possibleVals.end(); ++it)
            {
                if ((*it).name == "ε")
                    std::cout << "something else"; // FIXME (we want FOLLOW of last nonterminal)
                else
                    std::cout << (*it).name;
                // if not last
                if (*it != *possibleVals.rbegin())
                {
                    std::cout << " or ";
                }
            }

            std::cout << std::endl;

            // std::cout << a->posStr() << ": Skipping token " << a->lexeme << std::endl;

            a = lexer.next();
        }
        // else if ( M[X,a] = X->Y1Y2...Yk )
        else
        {
            parseStack.pop();
            std::vector<Util::Symbol> vec;

            if (a->lexeme != "$")
                vec = parseTable->get(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL));
            else
                vec = parseTable->get(*X, Util::Symbol(a->lexeme, Util::SymbolType::END));

            // std::cout << a->lexeme;
            // std::cout << " expand: ";
            // std::cout << X->name << " --> ";
            // for (auto &x : vec)
            // {
            //     // output production X->Y1Y2...Yk
            //     std::cout << x.name << " ";
            // }
            // std::cout << std::endl;

            currentPNode->addChildren(vec);

            int idx = vec.size() - 1;
            for (auto it = vec.rbegin(); it != vec.rend(); ++it)
            {
                // push Yk,Yk-1,...Y1 onto the stack, with Y1 on top
                parseStack.push(std::make_pair(new Util::Symbol(it->name, it->getType()), currentPNode->getChildren()[idx]));
                idx--;
            }
        }

        X = parseStack.top().first;
        currentPNode = parseStack.top().second;
    }

    return tree;
}
