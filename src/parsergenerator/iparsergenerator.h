#pragma once

#include <fstream>

#include <filesystem>
#include <string>
#include "util/parsetree.h"

namespace ParseGen::Parser
{
    class IParserGenerator;
    class IParserGenerator
    {
    protected:
        std::string language;
        std::string templateSubDir;

    public:
        std::string bnf;
        virtual Util::ParseTree *parse(std::string input) = 0;
        virtual std::string codeToString() = 0;

        void codeToFile()
        {
            if (templateSubDir == "")
                throw;
            
            auto templateDir = std::filesystem::current_path() / "templates" / templateSubDir;
            auto targetDir = std::filesystem::current_path() / "target"; // TODO: take as input!

            for (const auto &entry : std::filesystem::directory_iterator(targetDir))
                std::filesystem::remove_all(entry.path());

            std::filesystem::copy(templateDir, targetDir, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::recursive);

            std::string code = this->codeToString();

            std::ofstream outTemplate(targetDir / "template.cpp");

            outTemplate << code;
            outTemplate.close();
        };

        // required, but not implemented here
        // static IParserGenerator *Create(std::string language, std::string bnf);
    };
}
