#pragma once

#include <functional>
#include <string>

#include "iparsergenerator.h"
#include "llpred/parsergenerator.h"
#include "lexyacc/parsergenerator.h"
#include "bnf/parsergenerator.h"

namespace ParseGen::Parser
{

    class ParserGeneratorFactory;
    class ParserGeneratorFactory
    {
    private:
        ParserGeneratorFactory()
        {
            Register("BNF", &BNF::BNFParserGenerator::Create);
            Register("LL(1) Predictive Parser", &LLPredictiveParser::LLPredictiveParserGenerator::Create);
            Register("LEX/YACC", &Yacc::LexYaccGenerator::Create);
        };

        ParserGeneratorFactory(const ParserGeneratorFactory &){};

        ParserGeneratorFactory &operator=(const ParserGeneratorFactory &) { return *this; }

        // lang, pair: func
        std::map<std::string, IParserGenerator *(*)(std::string, std::string)> m_FactoryMap;
        const char **options;

    public:
        static ParserGeneratorFactory *Get()
        {
            static ParserGeneratorFactory instance;
            return &instance;
        }

        void Register(const std::string &type, IParserGenerator *(*pfnCreate)(std::string, std::string))
        {
            m_FactoryMap[type] = pfnCreate;

            delete options;
            options = new const char *[m_FactoryMap.size()];
            int i = 0;
            for (auto it = m_FactoryMap.begin(); it != m_FactoryMap.end(); ++it)
            {
                options[i] = (*it).first.c_str();
                ++i;
            }
        };

        IParserGenerator *create(const std::string &type, const std::string &lang, const std::string &bnf)
        {
            auto it = m_FactoryMap.find(type);
            if (it != m_FactoryMap.end())
                return it->second(lang, bnf);
            return NULL;
        }

        static IParserGenerator *Create(const std::string &type, const std::string &lang, const std::string &bnf)
        {
            auto instance = Get();
            return instance->create(type, lang, bnf);
        }

        const char **getOutputTypes() const
        {
            return this->options;
        }

        static const char **GetOutputTypes()
        {
            auto instance = Get();
            return instance->getOutputTypes();
        }

        int size() const
        {
            return m_FactoryMap.size();
        }

        static int Size()
        {
            auto instance = Get();
            return instance->size();
        }
    };
}
