#include "maingui.h"
#include "nonterminal.h"

#include <iostream> // FIXME
#include "parsergenerator/factory.h"
#include "parsergenerator/util/parsetree.h"
#include "parsetreerenderer.h"

namespace ParseGen::GUI
{
    void MainEditor::framebuffer_size_callback(GLFWwindow *window, int width, int height)
    {
        glViewport(0, 0, width, height);
        m_WindowWidth = width;
        m_WindowHeight = height;
    }

    void MainEditor::teardown(GLFWwindow *window)
    {
        if (window != NULL)
        {
            glfwDestroyWindow(window);
        }
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwTerminate();
    }

    GLFWwindow *MainEditor::initGLFW(std::string programName, int width, int height)
    {
        if (!glfwInit())
        {
            std::cerr << "[ERROR] Could not initialise GLFW!" << std::endl;
            return NULL;
        }
        else
        {
            std::cout << "[INFO] GLFW initialised" << std::endl;
        }

        // setup GLFW
        glfwWindowHint(GLFW_DOUBLEBUFFER, 1);
        glfwWindowHint(GLFW_DEPTH_BITS, 24);
        glfwWindowHint(GLFW_STENCIL_BITS, 8);

        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        std::string glsl_version;

        glsl_version = "#version 150";
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

        GLFWwindow *window = glfwCreateWindow(width, height, programName.c_str(), NULL, NULL);

        if (!window)
        {
            std::cerr << "[ERROR] Couldn't create GLFW Window" << std::endl;
            teardown(NULL);
            return NULL;
        }
        else
        {
            std::cout << "[INFO] GLFW Window created" << std::endl;
        }

        glfwMakeContextCurrent(window);

        // VSync
        glfwSwapInterval(1);

        glewExperimental = GL_TRUE;

        if (glewInit() != GLEW_OK)
        {
            std::cerr << "[ERROR] Couldn't initialise GLEW" << std::endl;
            teardown(NULL);
            return NULL;
        }
        else
        {
            std::cout << "[INFO] GLEW initialised" << std::endl;
        }

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // ImGui::StyleColorsLight();
        // ImGui::StyleColorsClassic();
        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version.c_str());

        return window;
    }

    void MainEditor::Start()
    {
        GLFWwindow *window = initGLFW(m_WindowTitle, m_WindowWidth, m_WindowHeight);
        if (!window)
            return;

        std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

        int actualWindowWidth, actualWindowHeight;
        glfwGetWindowSize(window, &actualWindowWidth, &actualWindowHeight);
        glViewport(0, 0, actualWindowWidth, actualWindowHeight);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

        float lastTime = glfwGetTime();

        // custom font because epsilon sadness
        ImGuiIO &io = ImGui::GetIO();
        ImVector<ImWchar> fontRanges;
        ImFontGlyphRangesBuilder builder;
        builder.AddText(u8"ε");
        // builder.AddRanges(io.Fonts->GetGlyphRangesDefault());
        builder.BuildRanges(&fontRanges);

        io.Fonts->AddFontFromFileTTF("fonts/Inconsolata-Medium.ttf", 15.0f, NULL, io.Fonts->GetGlyphRangesDefault());
        ImFontConfig config;
        config.MergeMode = true;
        io.Fonts->AddFontFromFileTTF("fonts/Roboto-Medium.ttf", 15.0f, &config, fontRanges.Data);
        io.Fonts->Build();

        do
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            float currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            RenderFrame(deltaTime);

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSwapBuffers(window);
            glfwPollEvents();
        } while (!glfwWindowShouldClose(window));

        teardown(window);
    }

    void MainEditor::deleteRule(int index)
    {
        m_Rules.erase(m_Rules.begin() + index);
        // TODO: can we delete the non-terminal too?
    }

    void MainEditor::addRule(std::string name)
    {
        NonTerminal *symbol;
        // check if this non-terminal already exists - if not, create!
        bool nonTermExists = false;
        for (auto nonTerm : NonTerminal::allNonTerminals)
        {
            if (name == nonTerm->name)
            {
                nonTermExists = true;
                symbol = nonTerm;
                break;
            }
        }

        // only create rules for unique non-terminals
        // all non-terminals should have rules - so this is valid here
        // the inverse (creating non-term on the canvas) will be handled
        // on that side of the code using this function.
        // TODO: check we actually comply with the above ^
        if (!nonTermExists)
        {
            symbol = new NonTerminal(name);
        }

        m_Rules.push_back(Rule(symbol));
    }

    int MainEditor::rulesCollapseGui()
    {
        static int selected = -1;
        static int toDelete = -1;
        for (int n = 0; n < m_Rules.size(); n++)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));

            if (ImGui::Button(("X##" + std::to_string(n)).c_str()))
            {
                toDelete = n;
                ImGui::OpenPopup("Are You Sure?##deleteRule");
            }

            ImGui::PopStyleColor();
            ImGui::SameLine();

            if (ImGui::Selectable((m_Rules[n].getName() + "##" + std::to_string(m_Rules[n].id)).c_str(), selected == n))
            {
                selected = n;
            }
        }

        if (ImGui::Button("+ Add Rule"))
        {
            ImGui::OpenPopup("New Rule");
        }

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("New Rule", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            static char inputText[128];
            // TODO: InputTextComboBox for non-terminals
            ImGui::InputTextWithHint("##rulename", "rule name", inputText, IM_ARRAYSIZE(inputText));

            if (ImGui::Button("OK", ImVec2(120, 0)))
            {
                // append a new rule
                addRule(std::string(inputText));
                // reset string
                memset(inputText, 0, sizeof inputText);
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0)))
            {
                memset(inputText, 0, sizeof inputText);
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        if (ImGui::BeginPopupModal("Are You Sure?##deleteRule", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            if (ImGui::Button("Yes", ImVec2(120, 0)))
            {
                // delete rule
                deleteRule(toDelete);
                toDelete = -1;
                selected = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::SameLine();
            if (ImGui::Button("No", ImVec2(120, 0)))
            {
                toDelete = -1;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        return selected;
    }

    void MainEditor::nonTerminalsCollapseGui()
    {
        static int selected = -1;
        for (int n = 0; n < NonTerminal::allNonTerminals.size(); n++)
        {
            if (ImGui::Selectable(NonTerminal::allNonTerminals[n]->name.c_str(), selected == n))
            {
                selected = n;
            }
        }
    }

    // void MainEditor::terminalsCollapseGui(int selectedRule)
    // {
    //     static int selected = -1;
    //     int n = 0;
    //     for (auto it = m_Rules[selectedRule].m_Terminals.begin(); it != m_Rules[selectedRule].m_Terminals.begin(); ++it)
    //     {
    //         if (it->second > 0)
    //         {
    //             if (ImGui::Selectable(NonTerminal::allNonTerminals[n]->name.c_str(), selected == n))
    //             {
    //                 selected = n;
    //             }
    //         }
    //         ++n;
    //     }
    //     for (int n = 0; n < m_Rules[selectedRule].m_Terminals.size(); n++)
    //     {
    //
    //         if (ImGui::Selectable(NonTerminal::allNonTerminals[n]->name.c_str(), selected == n))
    //         {
    //             selected = n;
    //         }
    //     }
    // }

    void MainEditor::drawTree(Parser::Util::ParseTree *parseTree)
    {
        auto renderer = ParseGen::GUI::ParseTree::Renderer::Get();
        renderer->render(parseTree);
    }

    void MainEditor::ruleTesterWindow(bool *p_open)
    {
        static ImGuiWindowFlags flags = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        static Parser::Util::ParseTree *parseTree;

        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        if (ImGui::Begin("Rule Tester", p_open, flags))
        {
            ImGui::BeginChild("Sample Rules To Test", ImVec2(viewport->WorkSize.x / 4, 0), true);
            ImGui::Text("Sample Inputs for Test");

            static int sampleInputsIdx = -1;
            static int toDeleteIdx = -1;
            static char inputText[1024 * 1024];

            ImGui::BeginChild("##Content", ImVec2(0, -125), false, 0);
            ImGui::PushItemWidth(ImGui::GetWindowSize().x);
            if (ImGui::BeginListBox("##sampleinputs"))
            {
                for (int n = 0; n < this->inputs.size(); n++)
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));

                    if (ImGui::Button(("X##input" + std::to_string(n)).c_str()))
                    {
                        toDeleteIdx = n;
                        ImGui::OpenPopup("Are You Sure?##deleteInput");
                    }

                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    const bool is_selected = (sampleInputsIdx == n);
                    if (ImGui::Selectable(this->inputs[n].c_str(), is_selected))
                    {
                        sampleInputsIdx = n;
                        if (this->parser != nullptr)
                            parseTree = this->parser->parse(inputs[sampleInputsIdx]);
                    }
                }
            }

            if (ImGui::BeginPopupModal("Are You Sure?##deleteInput", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                if (ImGui::Button("Yes", ImVec2(120, 0)))
                {
                    // delete rule
                    inputs.erase(inputs.begin() + toDeleteIdx);
                    toDeleteIdx = -1;
                    sampleInputsIdx = -1;
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("No", ImVec2(120, 0)))
                {
                    toDeleteIdx = -1;
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndListBox();
            ImGui::EndChild();

            ImGui::BeginChild("##Footer", ImVec2(0, 0), false, 0);
            if (ImGui::Button("+ Add Input", ImVec2(ImGui::GetWindowSize().x / 2, 0)))
            {
                ImGui::OpenPopup("Add Input");
            }

            if (inputs.size() > 0)
            {
                ImGui::SameLine();
                if (ImGui::Button("Edit Input", ImVec2(ImGui::GetWindowSize().x / 2, 0)))
                {
                    memset(inputText, 0, sizeof inputText);
                    memcpy(inputText, inputs[sampleInputsIdx].c_str(), inputs[sampleInputsIdx].size());
                    ImGui::OpenPopup("Edit Input");
                }
            }

            // TODO: more options
            static int langIdx = 0;
            static int typeIdx = 0;
            const char **outputTypes;
            outputTypes = Parser::ParserGeneratorFactory::Get()->getOutputTypes();
            const char *outputLangs[] = {"C++", "BNF"};
            static int langStart = 0;
            static int langEnd = IM_ARRAYSIZE(outputLangs);

            ImGui::PushItemWidth(ImGui::GetWindowSize().x);
            ImGui::Combo("##outputTypes", &typeIdx, outputTypes, Parser::ParserGeneratorFactory::Get()->size(), -1);
            ImGui::Combo("##outputLangs", &langIdx, outputLangs + langStart, langEnd - langStart + 1, -1);
            ImGui::PopItemWidth();

            // control conditional visibility of options
            if (outputTypes[typeIdx] == "BNF")
            {
                langStart = 1;
                langEnd = 1;
                langIdx = 0;
            }
            else
            {
                langStart = 0;
                langEnd = 1;
            }

            if (ImGui::Button("Compile and Generate Output", ImVec2(ImGui::GetWindowSize().x, 0)))
            {
                std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
                // create new parser with given options
                this->parser = processRules(outputTypes[typeIdx], outputLangs[langIdx]);
                if (sampleInputsIdx >= 0)
                    parseTree = this->parser->parse(inputs[sampleInputsIdx]);
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                std::cout << "Time diff = " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count() << "[ns]" << std::endl;
            }

            if (this->parser)
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImU32)ImColor(25, 107, 22));
            if (ImGui::Button("Output Code", ImVec2(ImGui::GetWindowSize().x, 0)))
            {
                if (this->parser)
                    this->parser->codeToFile();
            }
            if (this->parser)
                ImGui::PopStyleColor();

            if (ImGui::BeginPopupModal("Add Input", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputTextMultiline("##inputdata", inputText, IM_ARRAYSIZE(inputText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput | ImGuiInputTextFlags_AlwaysInsertMode);

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    // append a new rule
                    inputs.push_back(std::string(inputText));
                    sampleInputsIdx = inputs.size() - 1;
                    if (this->parser != nullptr)
                        parseTree = this->parser->parse(inputs[sampleInputsIdx]);

                    // reset string
                    memset(inputText, 0, sizeof inputText);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    memset(inputText, 0, sizeof inputText);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            if (ImGui::BeginPopupModal("Edit Input", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::InputTextMultiline("##inputdataedit", inputText, IM_ARRAYSIZE(inputText), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);

                if (ImGui::Button("OK", ImVec2(120, 0)))
                {
                    // append a new rule
                    inputs[sampleInputsIdx] = std::string(inputText);
                    if (this->parser != nullptr)
                        parseTree = this->parser->parse(inputs[sampleInputsIdx]);
                    // reset string
                    memset(inputText, 0, sizeof inputText);
                    ImGui::CloseCurrentPopup();
                }
                ImGui::SetItemDefaultFocus();
                ImGui::SameLine();
                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    memset(inputText, 0, sizeof inputText);
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::EndChild();
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("Output", ImVec2(0, 0), true);

            if (this->parser != nullptr)
            {
                if (inputs.size() == 0 || outputLangs[langIdx] == "BNF")
                {
                    ImGui::TextWrapped(parser->bnf.c_str());
                }
                else
                {
                    if (parseTree->isStub())
                    {
                        // text output only :)
                        ImGui::TextWrapped(parser->bnf.c_str());
                    }
                    else
                    {
                        drawTree(parseTree);
                    }
                }
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void MainEditor::RenderFrame(float deltaTime)
    {
        // ImGui::ShowDemoWindow();

        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        m_WindowWidth = viewport->WorkSize.x;
        m_WindowHeight = viewport->WorkSize.y;

        int windowSplit = 5;

        ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(m_WindowWidth / windowSplit, m_WindowHeight));
        ImGui::Begin("##Window", nullptr, windowFlags);

        int selected = -1;

        ImGui::BeginChild("##Content", ImVec2(0, -25), false, 0);
        if (ImGui::CollapsingHeader("Rules"))
        {
            selected = rulesCollapseGui();
        }

        if (ImGui::CollapsingHeader("Non-Terminals"))
        {
            nonTerminalsCollapseGui();
        }
        ImGui::EndChild();

        // if (ImGui::CollapsingHeader("Terminals"))
        // {
        //     terminalsCollapseGui(selected);
        // }

        ImGui::BeginChild("##Footer", ImVec2(0, 0), false, 0);

        static bool showRuleTester = false;
        if (ImGui::Button("Generate & Test Parser", ImVec2(ImGui::GetWindowSize().x, 0)))
        {
            showRuleTester = true;
        }
        ImGui::EndChild();

        ImGui::End();

        //
        if (showRuleTester)
        {
            ruleTesterWindow(&showRuleTester);
        }

        if (selected >= 0)
        {
            ImGui::SetNextWindowPos(ImVec2(m_WindowWidth / windowSplit, 0));
            ImGui::SetNextWindowSize(ImVec2((windowSplit - 1) * m_WindowWidth / windowSplit, m_WindowHeight));
            ImGui::Begin("##NodeEditorWindow", nullptr, windowFlags);
            std::string newRule = m_Rules[selected].RenderFrame();
            if (newRule != "")
                addRule(newRule);
            ImGui::End();
        }
    }

    Parser::IParserGenerator *MainEditor::processRules(std::string outputType, std::string outputLang)
    {
        // all generators require BNF as input
        std::string bnf = "";
        for (auto rule : m_Rules)
        {
            bnf += rule.BNF();
            bnf += "\n";
        }

        // FIXME
        // bnf = "E ::= T E'\nT ::= F T'\nT' ::= \"*\" F T' | ε\nE' ::= \"+\" T E' | ε\nF ::= \"id\" | \"(\" E \")\"";
        // bnf = "object ::= \"{\" kvpair kvpair2 \"}\"\nkvpair ::= string \":\" value\nkvpair2 ::= \",\" kvpair kvpair2 | ε\nstring ::= \"\"\" charsolo charloop \"\"\"\nvalue ::= array | object | number | string | \"true\" | \"false\" | \"null\"\narray ::= \"[\" value value2 \"]\"\nvalue2 ::= \",\" value value2 | ε\nnumber ::= \"-\" \"0\" fraction | \"-\" \"0\" fraction exponent | \"-\" \"0\" exponent | \"-\" digit digitloop | \"-\" digit digitloop fraction | \"-\"  digit digitloop fraction exponent | \"-\"  digit digitloop exponent | \"0\" | digit digitloop | digit digitloop fraction | digit digitloop fraction exponent | digit digitloop exponent\nfraction ::= \".\" digitwithzero digitloop\ndigit ::= \"1\" | \"2\" | \"3\" | \"4\" | \"5\"\ndigitwithzero ::= \"0\" | digit\ndigitloop ::= digitwithzero digitloop | ε\nexponent ::= \"e\" digitwithzero digitloop | \"e\" \"-\" digitwithzero digitloop | \"e\" \"+\" digitwithzero digitloop | \"E\" digitwithzero digitloop | \"E\" \"-\" digitwithzero digitloop | \"E\" \"+\" digitwithzero digitloop\ncharsolo ::= \"a\" | \"b\" | \"c\" | \"x\" | \"y\" | \"z\"\ncharloop ::= charsolo charloop | ε";

        auto parser = Parser::ParserGeneratorFactory::Create(outputType, outputLang, bnf);
        return parser;
    }

    MainEditor::~MainEditor()
    {
        // memory freeing!
        for (auto nonTerm : NonTerminal::allNonTerminals)
            delete nonTerm;
    }
}
