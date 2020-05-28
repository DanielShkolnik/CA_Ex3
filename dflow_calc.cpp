/* 046267 Computer Architecture - Spring 2020 - HW #3 */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
class Node{
public:
    int inst_num;
    unsigned int opcode;
    unsigned int longestPath;
    Node* left;
    Node* right;
    Node* parent;
    Node(int inst_num ,unsigned int opcode):inst_num(inst_num) ,opcode(opcode) ,longestPath(0),left(nullptr) ,right(nullptr) ,parent(nullptr){}
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
    ~Exit(){
        exitNode* current=this->exitList;
        exitNode* temp;
        while(current!= nullptr){
            temp=current;
            current=current->next;
            delete temp;
        }
    }
};

class RegInfo{
public:
    unsigned int instPos;
    Node* inst;
    bool isUsed;
    RegInfo():instPos(0) ,inst(nullptr),isUsed(false){}
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
        Node root(0 ,9999);
        this->entry = &root;
        Exit tail;
        this->exit = &tail;
        for(unsigned int i = 0 ; i < this->numOfInsts ; i++){
            nodePtrArry[i] = nullptr;
        }
    };
    ~Analyzer(){
        for(unsigned int i=0; i<this->numOfInsts; i++){
            delete this->nodePtrArry[i];
        }
        delete[] nodePtrArry;
    };
    void addToExit(Node* element){
        element->left = this->entry;
        element->right = this->entry;
        element->longestPath = opsLatency[element->opcode];
        if(this->exit->last == nullptr){
            exitNode* head = new exitNode(element);
            this->exit->exitList = head;
            this->exit->last = head;
        }else{
            exitNode* nextNode = new exitNode(element);
            this->exit->last->next = nextNode;
            nextNode->prev = this->exit->last;
            this->exit->last = nextNode;
        }
    }

    void removeFromExit(Node* element){
        exitNode* current = this->exit->exitList;
        while(current != nullptr){
            if(current->node->inst_num == element->inst_num){
                if(current == this->exit->last && current == this->exit->exitList){
                    this->exit->last = nullptr;
                    this->exit->exitList = nullptr;
                }
                else if(current == this->exit->last){
                    this->exit->last = current->prev;
                    current->prev->next = current->next;
                }
                else if(current == this->exit->exitList){
                    this->exit->exitList = current->next;
                    current->next->prev = current->prev;
                }
                else{
                    current->next->prev = current->prev;
                    current->prev->next = current->next;
                }
                delete current;
                return;
            }
            current = current->next;
        }
    }

    void addDependency(Node* parent ,Node* son, bool isLeft){
        if(isLeft){
            if(parent->left == this->entry) {
                parent->left = son;
                son->parent = parent;
                removeFromExit(son);
                if(parent->right != this->entry){
                    if(son->longestPath > parent->right->longestPath) parent->longestPath += (son->longestPath - parent->right->longestPath);
                }
                else parent->longestPath += son->longestPath;
                return;
            }
        }
        else{
            if(parent->right == this->entry){
                parent->right = son;
                son->parent = parent;
                removeFromExit(son);
                if(parent->left != this->entry){
                    if(son->longestPath > parent->left->longestPath) parent->longestPath += (son->longestPath - parent->left->longestPath);
                }
                else parent->longestPath += son->longestPath;
                return;
            }
        }
    }

    void computeDataFlowGraph() {
        RegInfo regs[32];
        for (unsigned int i = 0; i < this->numOfInsts; i++) {
            const InstInfo current = progTrace[i];
            Node *node = new Node(i, current.opcode);
            addToExit(node);

            if (regs[current.src1Idx].isUsed && regs[current.src2Idx].isUsed) {
                addDependency(node, regs[current.src1Idx].inst,true);
                addDependency(node, regs[current.src2Idx].inst,false);
            } else if (regs[current.src1Idx].isUsed) {
                addDependency(node, regs[current.src1Idx].inst,true);
            } else if (regs[current.src2Idx].isUsed) {
                addDependency(node, regs[current.src2Idx].inst,false);
            }
            regs[current.dstIdx].isUsed = true;
            regs[current.dstIdx].instPos = i;
            regs[current.dstIdx].inst = node;
            this->nodePtrArry[i] = node;
        }
    }

    int getInstDepth(unsigned int i){
        if(i>=this->numOfInsts) return -1;
        Node* node = this->nodePtrArry[i];
        if(node == nullptr){
            return -1;
        }
        return node->longestPath - opsLatency[node->opcode];
    }

    int getProgDepth(){
        exitNode* current = this->exit->exitList;
        unsigned int max_depth = 0;
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
    }catch(std::bad_alloc& e){
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
    if(analyzer->numOfInsts <= theInst) return -1;
    return analyzer->getInstDepth(theInst);
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    if(ctx == nullptr || src1DepInst == nullptr || src2DepInst == nullptr) return -1;
    Analyzer *analyzer = (Analyzer*) ctx;
    if(analyzer->numOfInsts <= theInst) return -1;

    if(analyzer->nodePtrArry[theInst]->left == analyzer->entry && analyzer->nodePtrArry[theInst]->right == analyzer->entry) {
        *src1DepInst = -1;
        *src2DepInst = -1;
    } else if(analyzer->nodePtrArry[theInst]->left == analyzer->entry){
        *src1DepInst = -1;
        *src2DepInst = analyzer->nodePtrArry[theInst]->right->inst_num;
    }
    else if(analyzer->nodePtrArry[theInst]->right == analyzer->entry){
        *src2DepInst = -1;
        *src1DepInst = analyzer->nodePtrArry[theInst]->left->inst_num;
    }
    else{
        *src1DepInst = analyzer->nodePtrArry[theInst]->left->inst_num;
        *src2DepInst = analyzer->nodePtrArry[theInst]->right->inst_num;
    }
    return 0;
}

int getProgDepth(ProgCtx ctx) {
    if(ctx == nullptr) return -1;
    Analyzer *analyzer = (Analyzer*) ctx;
    return analyzer->getProgDepth();
}