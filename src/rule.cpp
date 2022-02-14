#include "rule.h"
#include "node.h"
#include "nonterminal.h"
#include "nonterminalnode.h"

#include <iostream> // FIXME
#include <string>
#include <algorithm>
#include <map>
#include <set>

namespace ParseGen::GUI
{
    std::string Rule::getName()
    {
        return this->symbol->name;
    }

    uint Rule::getNonTermId()
    {
        return this->symbol->id;
    }

    Rule::Rule(NonTerminal *symbol) : symbol(symbol), id(currentId++)
    {
        ed::Config config;
        // TODO: stored state (see below, but this created weird filesystem artefacts)
        // config.SettingsFile = ("parsegen" + std::to_string(id) + ".json").c_str();
        m_Context = ed::CreateEditor(&config);

        // start and end nodes!
        m_StartNode = new SpecialNode(true);
        m_EndNode = new SpecialNode(false);
    };

    std::string Rule::RenderFrame()
    {
        ed::SetCurrentEditor(m_Context);
        ed::Begin("My Editor", ImVec2(0.0f, 0.0f));

        std::string newRuleName = "";

        if (m_FirstFrame)
        {
            auto windowSize = ImGui::GetWindowSize();
            ed::SetNodePosition(m_StartNode->nodeId, ImVec2(windowSize.x / 6 - windowSize.x / 8, windowSize.y / 5));
            ed::SetNodePosition(m_EndNode->nodeId, ImVec2(4 * windowSize.x / 5, windowSize.y / 5));
            m_FirstFrame = false;
        }

        m_StartNode->RenderFrame();

        for (Node *node : m_Nodes)
        {
            node->RenderFrame();
        }
        m_EndNode->RenderFrame();

        for (auto *link : m_Links)
            ed::Link(link->linkId, link->endNode->inPin, link->startNode->outPin, ImColor(0.0f, 1.0f, 0.0f), 2.0f);

        static ImVec2 newNodePosition;
        if (ed::IsBackgroundDoubleClicked())
        {
            ImGui::OpenPopup("Create New Node");
            newNodePosition = ImGui::GetMousePos();
        }

        if (ed::BeginCreate())
        {
            ed::PinId inputPinId, outputPinId;
            if (ed::QueryNewLink(&inputPinId, &outputPinId))
            {
                if (inputPinId && outputPinId)
                {
                    if (ed::AcceptNewItem())
                    {
                        Node *inNode = nullptr;
                        Node *outNode = nullptr;

                        if (m_EndNode->inPin.Get() == outputPinId.Get())
                        {
                            inNode = m_EndNode;
                        }
                        if (m_StartNode->outPin.Get() == inputPinId.Get())
                        {
                            outNode = m_StartNode;
                        }

                        for (auto *node : m_Nodes)
                        {
                            if (node->inPin.Get() == outputPinId.Get())
                            {
                                inNode = node;
                            }
                            if (node->outPin.Get() == inputPinId.Get())
                            {
                                outNode = node;
                            }
                        }

                        if (inNode && outNode)
                        {
                            outNode->outIsConnected = true;
                            inNode->inIsConnected = true;
                            outNode->numOutConnected += 1;
                            inNode->numInConnected += 1;

                            m_Links.push_back(new NodeLink(inNode, outNode));
                            ed::Link(m_Links.back()->linkId, m_Links.back()->endNode->inPin, m_Links.back()->startNode->outPin, ImColor(1.0f, 1.0f, 1.0f), 2.0f);
                        }
                        else
                        {
                            // at this point, we've done something like connect output to output
                            // so creation should be blocked
                        }
                    }
                }
            }
        }
        ed::EndCreate();

        if (ed::BeginDelete())
        {
            ed::LinkId deletedLinkId;
            while (ed::QueryDeletedLink(&deletedLinkId))
            {
                if (ed::AcceptDeletedItem())
                {
                    for (auto link = m_Links.begin(); link != m_Links.end(); ++link)
                    {
                        if ((*link)->linkId == deletedLinkId)
                        {
                            (*link)->startNode->numOutConnected -= 1;
                            if ((*link)->startNode->numOutConnected == 0)
                                (*link)->startNode->outIsConnected = false;

                            (*link)->endNode->numInConnected -= 1;
                            if ((*link)->endNode->numInConnected == 0)
                                (*link)->endNode->inIsConnected = false;

                            m_Links.erase(link);
                            break;
                        }
                    }
                }
            }

            ed::NodeId deletedNodeId;
            while (ed::QueryDeletedNode(&deletedNodeId))
            {
                if (ed::AcceptDeletedItem())
                {
                    for (auto node = m_Nodes.begin(); node != m_Nodes.end(); ++node)
                    {
                        if ((*node)->nodeId == deletedNodeId)
                        {
                            for (int i = 0; i < m_Links.size(); ++i)
                            {
                                if (m_Links[i]->endNode == *node || m_Links[i]->startNode == *node)
                                {
                                    m_Links[i]->startNode->numOutConnected -= 1;
                                    if (m_Links[i]->startNode->numOutConnected == 0)
                                        m_Links[i]->startNode->outIsConnected = false;

                                    m_Links[i]->endNode->numInConnected -= 1;
                                    if (m_Links[i]->endNode->numInConnected == 0)
                                        m_Links[i]->endNode->inIsConnected = false;

                                    m_Links.erase(m_Links.begin() + i);
                                    --i; // don't double-skip!
                                }
                            }

                            if (m_Terminals.find((*node)->name) != m_Terminals.end())
                            {
                                m_Terminals[(*node)->name]--;
                                m_Terminals[(*node)->name] = std::max(0, m_Terminals[(*node)->name]);
                            }

                            m_Nodes.erase(node);
                            break;
                        }
                    }
                }
            }
        }
        ed::EndDelete();

        ed::Suspend();
        if (ImGui::BeginPopupModal("Create New Node", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Name new node:");
            static char inputText[200];
            ImGui::InputText("##NodeInputName", inputText, 200);
            if (ImGui::Button("Cancel"))
            {
                memset(inputText, 0, sizeof inputText);
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            // TODO: InputTextComboBox
            if (ImGui::Button("Create Non-Terminal"))
            {
                NonTerminal *nodeSymbol;
                // check if this non-terminal already exists - if not, create!
                bool nonTermExists = false;
                for (auto nonTerm : NonTerminal::allNonTerminals)
                {
                    if (inputText == nonTerm->name)
                    {
                        nonTermExists = true;
                        nodeSymbol = nonTerm;
                        break;
                    }
                }

                if (!nonTermExists)
                {
                    // nodeSymbol = new NonTerminal(inputText);
                    newRuleName = std::string(inputText);
                    nodeSymbol = new NonTerminal(inputText);
                }

                m_Nodes.push_back(new NonTerminalNode(nodeSymbol));
                m_Nodes.back()->nodeId = ed::NodeId(m_Nodes.back()->id);
                m_Nodes.back()->inPin = ed::PinId(m_Nodes.back()->id + 1);
                m_Nodes.back()->outPin = ed::PinId(m_Nodes.back()->id + 2);

                ed::SetNodePosition(m_Nodes.back()->nodeId, newNodePosition);

                memset(inputText, 0, sizeof inputText);
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Create Terminal"))
            {
                m_Nodes.push_back(new TerminalNode(inputText));
                m_Nodes.back()->nodeId = ed::NodeId(m_Nodes.back()->id);
                m_Nodes.back()->inPin = ed::PinId(m_Nodes.back()->id + 1);
                m_Nodes.back()->outPin = ed::PinId(m_Nodes.back()->id + 2);

                ed::SetNodePosition(m_Nodes.back()->nodeId, newNodePosition);

                // if not in map yet, add it
                if (m_Terminals.find(m_Nodes.back()->name) == m_Terminals.end())
                {
                    m_Terminals[m_Nodes.back()->name] = 0;
                }

                m_Terminals[m_Nodes.back()->name]++;

                memset(inputText, 0, sizeof inputText);
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ed::Resume();

        ed::End();
        ed::SetCurrentEditor(nullptr);

        return newRuleName;
    }

    std::vector<std::vector<Node *>> printPaths(Node *start, Node *target, std::map<Node *, std::set<Node *>> *following, std::set<Node *> *visited, std::vector<Node *> prefix)
    {
        prefix.push_back(start);
        if (start == target)
        {
            // we've reached a valid path! return
            return std::vector<std::vector<Node *>>({std::vector<Node *>(prefix)});
        }

        auto retVal = std::vector<std::vector<Node *>>();

        for (auto child : (*following)[start])
        {
            for (auto path : printPaths(child, target, following, visited, prefix))
            {
                retVal.push_back(path);
            }
        }

        return retVal;
    }

    std::string Rule::BNF()
    {
        // lets keep an adjacency list:
        std::map<Node *, std::set<Node *>> following;

        // place all links into tree
        for (auto link : m_Links)
        {
            following[link->startNode].insert(link->endNode);
        }

        std::set<Node *> visited;

        auto productions = printPaths(this->m_StartNode, this->m_EndNode, &following, &visited, std::vector<Node *>());

        std::string bnf;

        bnf += this->getName();
        bnf += " ::= ";

        for (auto production : productions)
        {
            if (production.size() == 2)
            {
                // empty! output ε
                bnf += "ε ";
            }
            else
            {

                for (auto node : production)
                {
                    if (node == m_StartNode || node == m_EndNode)
                    {
                        continue;
                    }
                    else if (node->name[0] == '<')
                    {
                        // terminal
                        bnf += node->name.substr(1, node->name.size() - 2);
                        bnf += " ";
                    }
                    else
                    {
                        // non-terminal
                        bnf += "\"";
                        bnf += node->name;
                        bnf += "\" ";
                    }
                }
            }
            if (production != productions[productions.size() - 1])
                bnf += "| ";
        }

        return bnf;
    }

    // Rule::~Rule() {
    //     for (auto link : m_Links)
    //         delete link;

    //     for (auto node : m_Nodes)
    //         delete node;

    //     delete m_StartNode;
    //     delete m_EndNode;
    //     delete m_Context;

    //     delete symbol;
    // }
}
