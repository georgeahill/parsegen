%{
    #include <stdio.h>
    #include <stdarg.h>
    #include <stdlib.h>
    #include <string.h>
    void yyerror(char *s);
    int createnode(char* L, int argc, ...); // label L and descendants n1, n2 return index of new node

    void freeNodes();
    void printParseTree(int nodeIdx, char* prefix);

    int nodeNum = 1;
    
    typedef struct node {
        int id;
        char* label;
        int* children;
        int numChildren;
    } node;

    int nodeListSize = 2;
    node* nodeList;
%}

%%TEMPL_NEXT%%

int main(void) {
    nodeList = malloc(nodeListSize * sizeof(node));
    int retVal = yyparse();
    printParseTree(nodeNum-1, "");
    freeNodes();
    return retVal;
}

int createnode(char* label, int argc, ...) {
    if (nodeNum >= nodeListSize) {
        nodeListSize *= 2;
        nodeList = realloc(nodeList, nodeListSize * sizeof(node));
    }

    nodeList[nodeNum].id = nodeNum;
    nodeList[nodeNum].children = calloc(argc, sizeof(int));
    nodeList[nodeNum].numChildren = argc;
    nodeList[nodeNum].label = label;

    va_list arguments;
    va_start(arguments, argc);

    for (int x = 0; x < argc; x++)
    {
        int childIdx = va_arg(arguments, int);
        nodeList[nodeNum].children[x] = childIdx;
    }
    va_end(arguments);

    return nodeNum++;
}

void freeNodes() {
    for (int x = 0; x < nodeNum; ++x)
    {
        free(nodeList[x].children);
    }
    free(nodeList);
}

void printParseTree(int nodeIdx, char* prefix) {
    if (nodeIdx == 0) {
        // literal
        return;
    }

    printf("%s%s\n", prefix, nodeList[nodeIdx].label);
    
    char* newPrefix;
    newPrefix = malloc(strlen(prefix) + sizeof(char) + 1);

    strcpy(newPrefix, prefix);
    strcat(newPrefix, "\t");

    for (int i = 0; i < nodeList[nodeIdx].numChildren; ++i)
    {
        printParseTree(nodeList[nodeIdx].children[i], newPrefix);
    }
    
    free(newPrefix);

    // printf("\n");
}

void yyerror(char *s) {fprintf(stderr, "%s\n", s);}