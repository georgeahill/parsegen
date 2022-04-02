#pragma once

#include <vector>
#include <initializer_list>
#include <iostream>

#include "symbol.h"

#include <iostream> // FIXME

namespace Util
{
    class Node;
    class Node
    {
    public:
        Node(Symbol val) : value(val), parent(nullptr)
        {
            this->ancestor = this;
            this->y = 0.0f;
        };
        Node(Symbol val, Node *parent) : value(val), parent(parent)
        {
            this->ancestor = this;
            auto curPar = this;
            while (curPar != nullptr)
            {
                this->y++;
                curPar = curPar->parent;
            }
        };

        void addChildren(std::initializer_list<Symbol> children)
        {
            for (auto symbol : children)
            {
                this->children.push_back(new Node(symbol, this));
            }
        };

        void addChildren(std::vector<Symbol> children)
        {
            for (auto symbol : children)
            {
                this->children.push_back(new Node(symbol, this));
            }
        };

        void print(std::string prefix = "")
        {
            std::cout << prefix << value.name << std::endl;
            prefix = prefix + "\t";
            for (auto child : children)
            {
                child->print(prefix);
            }
        }

        friend std::ostream &operator<<(std::ostream &stream, const Node &w)
        {
            stream << w.value.name << ": x=" << w.x << " mod=" << w.mod;
            return stream;
        }

        std::string json()
        {
            /*

            {
                val: "E"
                type: "NonTerminal",
                children: [
                    {
                        val: "T",
                        type: "NonTerminal",
                        children: [
                            {
                                val: "id",
                                type: "Terminal"
                            }
                        ]
                    }
                ]
            }

            */

            std::string jsonStr = "{ \"val\": \"";
            jsonStr += this->value.name;
            jsonStr += "\", \"type\": \"";

            switch (this->value.getType())
            {
            case NON_TERMINAL:
                jsonStr += "NonTerminal";
                break;
            case TERMINAL:
                jsonStr += "Terminal";
                break;
            case EMPTY:
                jsonStr += "Empty";
                break;
            case END:
                jsonStr += "End";
                break;
            }

            jsonStr += "\"";

            if (children.size() > 0)
            {

                jsonStr += ", \"children\": [";

                int childrenCount = 0;
                for (auto child : children)
                {
                    jsonStr += child->json();
                    if (childrenCount < children.size() - 1)
                    {
                        jsonStr += ", ";
                    }
                    childrenCount++;
                }

                jsonStr += "]";
            }
            jsonStr += "}";

            return jsonStr;
        }

        std::vector<Node *> getChildren()
        {
            return children;
        }

        bool isLeaf()
        {
            return (this->children.size() == 0);
        }

        Node *leftBrother()
        {
            // TODO: use number() below
            Node *n = nullptr;

            if (this->parent != nullptr)
                for (auto node : this->parent->getChildren())
                {
                    if (node == this)
                        return n;
                    else
                        n = node;
                }

            return nullptr;
        }

        Node *leftmostSibling()
        {
            if (this->parent == nullptr)
                return nullptr;
            if (this->parent->getChildren()[0] == this)
                return nullptr;
            return this->parent->getChildren()[0];
        }

        int number()
        {
            if (this->parent != nullptr)
            {
                int n = 1;
                for (auto node : this->parent->getChildren())
                {
                    if (node == this)
                        return n;
                    else
                        ++n;
                }
            }
            return 1;
        }

        Node *left()
        {
            if (this->thread != nullptr)
                return this->thread;
            if (this->children.size() > 0)
                return this->children[0];
        }

        Node *right()
        {
            if (this->thread != nullptr)
                return this->thread;
            if (this->children.size() > 0)
                return this->children[this->children.size() - 1];
        }

        // for positioning algorithms etc.
        float x = -1;
        float y = 0;
        float mod = 0;
        float shift = 0;
        float change = 0;
        Node *thread = nullptr;
        Node *ancestor;

        Node *parent;
        Symbol value;

    protected:
        std::vector<Node *> children;
    };

    class ParseTree : public Node
    {
    public:
        ParseTree(Symbol val) : Node(val){};
        ParseTree(Symbol val, Node *parent) : Node(val, parent){};

        bool isStub()
        {
            return ((this->value.isEnd() || this->value.isEmpty()) && this->isLeaf());
        }

        std::string print()
        {
            if (this->isStub())
            {
                return this->value.name;
            }
            // else do nothing (since this is invalid)
            return "NotImplemented";
        }
    };
}
