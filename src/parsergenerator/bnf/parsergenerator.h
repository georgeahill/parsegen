#pragma once

#include "../util/parsetree.h"
#include "../util/symbol.h"

namespace ParseGen::Parser::BNF
{

    class BNFParserGenerator : public IParserGenerator
    {
    private:
        BNFParserGenerator(std::string bnf)
        {
            // BNF is only option!
            this->language = "BNF";
            this->bnf = bnf;
            this->templateSubDir = "bnf";
        };

    public:
        Util::ParseTree* parse(std::string input){
            // this format will just print text in output window :)
            return new Util::ParseTree({bnf, Util::SymbolType::END});
        };

        std::string codeToString(){
            return this->bnf;
        };

        static IParserGenerator *Create(std::string language, std::string bnf)
        {
            return new BNFParserGenerator(bnf);
        }
    };
}
