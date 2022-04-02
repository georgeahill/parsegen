#pragma once

#include <string>
#include <set>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "../iparsergenerator.h"
#include "../util/parseableinput.h"

#define GET_UCN(X) L## #X

namespace ParseGen::Parser::Yacc
{

    class LexYaccGenerator : public ::ParseGen::Parser::IParserGenerator
    {
    private:
        LexYaccGenerator(std::string language, std::string bnf)
        {
            this->language = language;
            this->bnf = bnf;
            this->templateSubDir = "lexyacc";
            this->rules = new Util::ParseableInput(bnf, ""); // just use first symbol as start
        };

        Util::ParseableInput *rules;

        std::string escaped_name(std::string name)
        {
            std::stringstream ss;

            for (auto ch : name)
            {
                if (isalnum(ch))
                {
                    ss << ch;
                }
                else
                {
                    ss << GET_UCN(ch);
                }
            }

            if (isdigit(ss.str()[0])) {
                return "t" + ss.str();
            }

            return ss.str();
        }

    public:
        Util::ParseTree *parse(std::string input)
        {
            // this format will just print text in output window :)
            return new Util::ParseTree({"YACC only supports code output for now.", Util::SymbolType::END});
        };

        std::string codeToString()
        {
            std::set<std::string> terminalSet;

            std::stringstream yaccPreamble;
            std::stringstream yaccRules;
            std::stringstream yaccAdditional;

            std::stringstream yacc;

            std::stringstream lexPreamble;
            std::stringstream lexRules;
            std::stringstream lexAdditional;

            std::stringstream lex;

            std::ifstream templ("target/yacctempl.y");

            std::string line;
            // read in preamble
            while (getline(templ, line))
            {
                if (line == "%%TEMPL_NEXT%%")
                    break;
                yaccPreamble << line << std::endl;
            }

            yaccPreamble << "\%start " << escaped_name(rules->start.name) << std::endl;

            lexPreamble << "%{\n\t#include \"y.tab.h\"\n%}" << std::endl;

            for (auto prod : rules->productions)
            {
                for (auto rule : prod.second)
                {
                    yaccRules << escaped_name(prod.first.name) << " : ";
                    for (auto val : rule)
                    {
                        if (val.isTerminal())
                        {
                            yaccRules << "tok_nt_" << escaped_name(val.name) << " ";
                            terminalSet.insert(val.name);
                        }
                        else if (val.isNonTerminal())
                        {
                            yaccRules << escaped_name(val.name) << " ";
                        }
                    }

                    yaccRules << "{ $$ = createnode(\"" << prod.first.name << "\", " << rule.size();

                    for (int i = 1; i <= rule.size(); ++i)
                    {
                        yaccRules << ", $" << i;
                    }

                    yaccRules << "); ";

                    if (prod.first.name == rules->start.name)
                    {
                        yaccRules << "YYACCEPT; ";
                    }

                    yaccRules << "}";

                    yaccRules << std::endl;
                }
            }

            // add default empty production
            yaccRules << "tok_nt_" << escaped_name("ε") << " : { $$ = createnode(\"ε\", 1, 0); } " << std::endl;

            for (auto terminal : terminalSet)
            {
                if (terminal == "ε") {
                    continue;
                }
                yaccRules << "tok_nt_" << escaped_name(terminal) << ": tok_" << escaped_name(terminal) << " { $$ = createnode(\"" << terminal << "\", 1, $1); }" << std::endl;
                yaccPreamble << "\%token tok_" << escaped_name(terminal);
                lexRules << "\"" << terminal << "\" { return tok_" << escaped_name(terminal) << "; }" << std::endl;
            }

            // read in remainder of template as additional functions
            while (getline(templ, line))
            {
                yaccAdditional << line << std::endl;
            }
            templ.close();

            lexRules << "[ \\t\\n] { ; }" << std::endl;
            lexAdditional << "int yywrap(void) { return 1;}" << std::endl;

            lex << lexPreamble.str() << std::endl;
            lex << "%%" << std::endl;
            lex << lexRules.str() << std::endl;
            lex << "%%" << std::endl;
            lex << lexAdditional.str() << std::endl;

            yacc << yaccPreamble.str() << std::endl;
            yacc << "%%" << std::endl;
            yacc << yaccRules.str() << std::endl;
            yacc << "%%" << std::endl;
            yacc << yaccAdditional.str() << std::endl;

            // TODO: move this up the stack a little (should happen in iparsergenerator)
            // this is only like this because it needs multiple templated files
            auto targetDir = std::filesystem::current_path() / "target";
            // std::filesystem::remove_all(targetDir / "yacctempl.y"); // delete now non-required template
            std::ofstream lexOut(targetDir / "parser.l");
            std::ofstream yaccOut(targetDir / "parser.y");

            lexOut << lex.str();
            yaccOut << yacc.str();

            lexOut.close();
            yaccOut.close();

            return "This file is not needed!";
        };

        static IParserGenerator *Create(std::string language, std::string bnf)
        {
            return new LexYaccGenerator(language, bnf);
        }
    };
}