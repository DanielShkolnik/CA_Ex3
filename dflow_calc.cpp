/* 046267 Computer Architecture - Spring 2020 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
class Node{
public:
    int inst_num;
    const InstInfo* inst;
    unsigned int longestPath;
    Node* left;
    Node* right;
    Node* parent;
    Node(int inst_num ,const InstInfo* inst):inst_num(inst_num) ,inst(inst) ,longestPath(0),left(nullptr) ,right(nullptr) ,parent(nullptr){}
};

class exitNode{
public:
    Node* node;
    exitNode* next;
    exitNode* prev;
    exitNode(Node* node):node(node) ,next(nullptr) ,prev(nullptr){}
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
        Node root(0 ,nullptr);
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
        element->right = this->entry;
        element->longestPath = opsLatency[element->inst->opcode];
        if(this->exit->last == nullptr){
            exitNode head(element);
            this->exit->exitList = &head;
            this->exit->last = &head;
        }else{
            exitNode nextNode(element);
            this->exit->last->next = &nextNode;
            nextNode.prev = this->exit->last;
            this->exit->last = &nextNode;
        }
    }

    void removeFromExit(Node* element){
        exitNode* current = this->exit->exitList;
        while(current != nullptr){
            if(current->node->inst == element->inst){
                current->prev->next = current->next;
                current->next->prev = current->prev;
                if(current == this->exit->last){
                    this->exit->last = current->prev;
                }
                if(current == this->exit->exitList){
                    this->exit->exitList = current->next;
                }
                if((current == this->exit->last)&&(current == this->exit->exitList)){
                    std::cout << "*********** ERROR ************" << std::endl;
                }
                return;
                //////delete
            }
            current = current->next;
        }
    }

    void addDependency(Node* parent ,Node* son){

        if(parent->left == this->entry){
            parent->left = son;
            son->parent = parent;
            removeFromExit(son);
            parent->longestPath += son->longestPath;
            return;
        }else if(parent->right == this->entry){
            parent->right = son;
            son->parent = parent;
            removeFromExit(son);
            if(son->longestPath > parent->left->longestPath){
                parent->longestPath += (son->longestPath - parent->left->longestPath);
            }
            return;
        }
        std::cout << "*********** ERROR ************" << std::endl;
    }

    void computeDataFlowGraph(){
        RegInfo regs[32];
        for(int i = 0 ; i < this->numOfInsts ; i++){
            const InstInfo current = progTrace[i];
            Node node(i ,&current);
            addToExit(&node);
            if(i <= regs[current.src1Idx].regTimer && i <= regs[current.src2Idx].regTimer){
                addDependency(&node ,regs[current.src1Idx].inst);
                addDependency(&node ,regs[current.src2Idx].inst);
                if(regs[current.src1Idx].regTimer > regs[current.src2Idx].regTimer) {
                    regs[current.dstIdx].regTimer = regs[current.src1Idx].regTimer + 1 + this->opsLatency[current.opcode];
                }else{
                    regs[current.dstIdx].regTimer = regs[current.src2Idx].regTimer + 1 + this->opsLatency[current.opcode];
                }
            }else if(i <= regs[current.src1Idx].regTimer){
                addDependency(&node ,regs[current.src1Idx].inst);
                regs[current.dstIdx].regTimer = regs[current.src1Idx].regTimer + 1 + this->opsLatency[current.opcode];
            }else if(i <= regs[current.src2Idx].regTimer) {
                addDependency(&node, regs[current.src2Idx].inst);
                regs[current.dstIdx].regTimer = regs[current.src2Idx].regTimer + 1 + this->opsLatency[current.opcode];
            }else {
                regs[current.dstIdx].regTimer = i + 1 + this->opsLatency[current.opcode];
            }
            regs[current.dstIdx].instPos = i;
            regs[current.dstIdx].inst = &node;
            this->nodePtrArry[i] = &node;
        }
    }

    int getInstDepth(int i){
        Node* node = this->nodePtrArry[i];
        if(node == nullptr){
            std::cout << "*********** ERROR ************" << std::endl;
            return -1;
        }
        return node->longestPath - opsLatency[node->inst->opcode];
    }

    int getProgDepth(){
        exitNode* current = this->exit->exitList;
        int max_depth = 0;
        while(current != nullptr){
            if(current->node->longestPath > max_depth){
                max_depth = current->node->longestPath;
            }
            current = current->next;
        }
        return max_depth;
    }
};

ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    try {
        Analyzer *analyzer = new Analyzer(opsLatency, progTrace, numOfInsts);
        analyzer->computeDataFlowGraph();
        return analyzer;
    }catch(std::bad_alloc&){
        return PROG_CTX_NULL;
    }
}

void freeProgCtx(ProgCtx ctx) {
    Analyzer *analyzer = (Analyzer*) ctx;
    delete analyzer;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    if(ctx == nullptr) return -1;
    Analyzer *analyzer = (Analyzer*) ctx;
    return analyzer->getInstDepth(theInst);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    if(ctx == nullptr || src1DepInst == nullptr || src2DepInst == nullptr) return -1;
    Analyzer *analyzer = (Analyzer*) ctx;
    if(analyzer->nodePtrArry[theInst]->left == analyzer->entry){
        *src1DepInst = -1;
    }else{
        *src1DepInst = analyzer->nodePtrArry[theInst]->left->inst_num;
    }
    if(analyzer->nodePtrArry[theInst]->right == analyzer->entry){
        *src2DepInst = -1;
    }else {
        *src2DepInst = analyzer->nodePtrArry[theInst]->right->inst_num;
    }
    return 0;
}

int getProgDepth(ProgCtx ctx) {
    if(ctx == nullptr) return -1;
    Analyzer *analyzer = (Analyzer*) ctx;
    return analyzer->getProgDepth();
}


