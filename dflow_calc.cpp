/* 046267 Computer Architecture - Spring 2020 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
class Node{
public:
    const InstInfo* inst;
    Node* left;
    Node* right;
    Node* parent;
    Node(const InstInfo* inst):inst(inst) ,left(nullptr) ,right(nullptr) ,parent(nullptr){}
};

class exitNode{
public:
    Node* inst;
    exitNode* next;
    exitNode* prev;
    exitNode(Node* inst):inst(inst) ,next(nullptr) ,prev(nullptr){}
};

class Exit{
public:
    exitNode* last;
    exitNode* exitList;
    Exit():last(nullptr) ,exitList(nullptr){}
};

class RegInfo{
public:
    int regTimer;
    int instPos;
    Node* inst;
    RegInfo():regTimer(0) ,instPos(-1) ,inst(nullptr){}

};

class Analyzer{
public:
    Node* entry;
    Exit* exit;
    const unsigned int* opsLatency;
    const InstInfo* progTrace;
    Node** nodePtrArry;
    unsigned int numOfInsts;
    Analyzer(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts):entry(nullptr) ,exit(
            nullptr) ,opsLatency(opsLatency) ,progTrace(progTrace) ,nodePtrArry(new Node*[numOfInsts]) ,numOfInsts(numOfInsts){
        Node root(nullptr);
        this->entry = &root;
        Exit tail;
        this->exit = &tail;
        addDependency(nullptr ,entry);
        for(int i = 0 ; i < this->numOfInsts ; i++){
            nodePtrArry[i] = nullptr;
        }
    };
    ~Analyzer(){
        delete[] nodePtrArry;
    };
    void addToExit(Node* element){
        element->left = this->entry;
        if(this->exit->last == nullptr){
            exitNode head(element);
            this->exit->exitList = &head;
            this->exit->last = &head;
        }else{
            exitNode nextNode(element);
            this->exit->last->next = &nextNode;
            this->exit->last = this->exit->last->next;
            this->exit->last->prev = this->exit->last;
        }
    }

    void addDependency(Node* parent ,Node* son){

        if(parent->left == this->entry){
            parent->left = son;
            son->parent = parent;
            son->left = this->entry;
            return;
        }
        if(parent->right == this->entry){
            parent->right = son;
            son->parent = parent;
            son->left = this->entry;
            return;
        }
        std::cout << "*********** ERROR ************" << std::endl;
    }

    void computeDataFlowGraph(){
        RegInfo regs[32];
        for(int i = 0 ; i < this->numOfInsts ; i++){
            const InstInfo current = progTrace[i];
            Node node(&current);
            if(i < regs[current.src1Idx].regTimer && i < regs[current.src2Idx].regTimer){
                addDependency(&node ,regs[current.src1Idx].inst);
                addDependency(&node ,regs[current.src2Idx].inst);
            }else if(i < regs[current.src1Idx].regTimer){
                addDependency(&node ,regs[current.src1Idx].inst);
            }else if(i < regs[current.src2Idx].regTimer){
                addDependency(&node ,regs[current.src2Idx].inst);
            }else{
                addToExit(&node);
            }
            regs[current.dstIdx].regTimer = i + this->opsLatency[current.opcode];
            regs[current.dstIdx].instPos = i;
            regs[current.dstIdx].inst = &node;
            this->nodePtrArry[i] = &node;
        }
    };
};

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    return PROG_CTX_NULL;
}

void freeProgCtx(ProgCtx ctx) {
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    return -1;
}

int getProgDepth(ProgCtx ctx) {
    return 0;
}


