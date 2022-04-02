#pragma once

#include <string>
#include <stack>
#include <sstream>

#include "../iparsergenerator.h"
#include "../util/parseableinput.h"
#include "../util/parsetree.h"
#include "../util/symbol.h"
#include "parsertable.h"
#include "lexer.h"
#include "token.h"

namespace ParseGen::Parser::LLPredictiveParser
{

    class LLPredictiveParserGenerator : public IParserGenerator
    {
    private:
        LLPredictiveParserGenerator(std::string language, std::string bnf)
        {
            this->language = language;
            this->templateSubDir = "llpred";
            this->bnf = bnf;
            this->rules = new Util::ParseableInput(bnf, ""); // just use first symbol as start
        };

        bool firstParse = true;
        Util::ParseableInput *rules;
        ParserTable *parseTable;
        std::string output;

        ParserTable *generateParseTable(Util::ParseableInput input)
        {
            // for each production of the grammar A->α
            //      for each terminal a in FIRST(α), add A->α to M[A,a]
            //      if ε is in FIRST(α), then for each terminal b in FOLLOW(A), add A->α to M[A,b]
            //      if ε is in FIRST(α) and $ is in FOLLOW(A), add A->α to M[A,$] too.
            //      if after all of the above, M[A,a] has no production at all, set it to error (which is normally just blank)

            ParserTable *parseTable = new ParserTable(input.start);

            parseTable->first = input.first;
            parseTable->follow = input.follow;

            for (auto prod : input.productions)
            {
                for (auto rule : prod.second)
                {
                    bool ruleContainsEmpty = false;
                    for (auto a : input.first[rule])
                    {
                        parseTable->insert(prod.first, a, rule);
                        if (a.isEmpty())
                            ruleContainsEmpty = true;
                    }

                    bool ruleContainsEnd = false;
                    if (ruleContainsEmpty)
                    {
                        // for each terminal b in FOLLOW(A), add to table
                        for (auto b : input.follow[prod.first])
                        {
                            parseTable->insert(prod.first, b, rule);
                            if (b.isEnd())
                                ruleContainsEnd = true;
                        }
                    }

                    if (ruleContainsEmpty && ruleContainsEnd)
                    {
                        parseTable->insert(prod.first, Util::Symbol("$", Util::SymbolType::END), rule);
                    }
                }

                for (auto follow : input.follow[prod.first])
                {
                    if (parseTable->isError(prod.first, follow))
                        parseTable->insert(prod.first, follow, std::vector<Util::Symbol>({Util::Symbol("synch", Util::SymbolType::SYNCH)}));
                }
            }

            return parseTable;
        }

    public:
        Util::ParseTree *parse(std::string input)
        {
            if (firstParse)
            {
                this->parseTable = generateParseTable(*rules);
                firstParse = false;
            }

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

            Util::ParseTree *tree = new Util::ParseTree(Util::Symbol(this->parseTable->start.name, Util::SymbolType::NON_TERMINAL));

            std::stack<std::pair<Util::Symbol *, Util::Node *>> parseStack;
            parseStack.push(std::make_pair(new Util::Symbol("$", Util::SymbolType::END), nullptr));
            parseStack.push(std::make_pair(new Util::Symbol(this->parseTable->start.name, Util::SymbolType::NON_TERMINAL), tree));

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
                else if ((this->parseTable->isSynch(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL)) && a->lexeme != "$") || (this->parseTable->isSynch(*X, Util::Symbol(a->lexeme, Util::SymbolType::END)) && a->lexeme == "$"))
                {
                    // error - synch (so skip top nonterminal)

                    parseStack.pop();
                    std::cout << a->posStr() << ": synch " << a->lexeme << std::endl;
                }
                else if ((this->parseTable->isError(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL)) && a->lexeme != "$") || (this->parseTable->isError(*X, Util::Symbol(a->lexeme, Util::SymbolType::END)) && a->lexeme == "$"))
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
                        vec = this->parseTable->get(*X, Util::Symbol(a->lexeme, Util::SymbolType::TERMINAL));
                    else
                        vec = this->parseTable->get(*X, Util::Symbol(a->lexeme, Util::SymbolType::END));

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

        std::string codeToString()
        {
            if (firstParse)
            {
                this->parseTable = generateParseTable(*rules);
                firstParse = false;
            }

            std::stringstream ss;

            std::ifstream templ("target/template.cpp");

            if (!templ.is_open())
            {
                // unable to open file! oops
                return "error";
            }

            std::string line;
            while (getline(templ, line))
            {
                ss << line << std::endl;
            }

            templ.close();

            ss << std::endl;

            ss << "int main(int argc, char **argv)" << std::endl;
            ss << "{" << std::endl;

            ss << this->parseTable->creationCode() << std::endl;

            ss << "    "
               << "Util::ParseTree *tree = parse(argv[1], parseTable);" << std::endl;
            ss << "    "
               << "std::cout << tree->json() << std::endl;" << std::endl;
            ss << "}" << std::endl;

            return ss.str();
        }

        static IParserGenerator *Create(std::string language, std::string bnf)
        {
            return new LLPredictiveParserGenerator(language, bnf);
        };
    };
}
