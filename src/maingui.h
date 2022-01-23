#pragma once

#include <string>
#include <vector>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui_internal.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/nodes/imgui_node_editor.h>

#include "rule.h"
#include "parsergenerator/iparsergenerator.h"

namespace ParseGen::GUI
{
    class MainEditor;
    class MainEditor
    {
    public:
        const char *m_WindowTitle;
        int m_WindowWidth, m_WindowHeight;

        std::vector<Rule> m_Rules;

        MainEditor(const char *windowTitle, int windowWidth, int windowHeight) : m_WindowTitle(windowTitle), m_WindowWidth(windowWidth), m_WindowHeight(windowHeight){};
        ~MainEditor();

        void Start();

        void RenderFrame(float deltaTime);
        void addRule(std::string name);

    private:
        GLFWwindow *initGLFW(std::string programName, int width, int height);
        void framebuffer_size_callback(GLFWwindow *window, int width, int height);
        void teardown(GLFWwindow *window);
        void deleteRule(int index);
        int rulesCollapseGui();
        void nonTerminalsCollapseGui();
        // void terminalsCollapseGui(int);
        void drawTree(Parser::Util::ParseTree *parseTree);
        void ruleTesterWindow(bool* open);

        Parser::IParserGenerator *processRules(std::string, std::string);

        std::vector<std::string> inputs;
        Parser::IParserGenerator* parser;
    };
}