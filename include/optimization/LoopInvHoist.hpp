#pragma once

#include <unordered_map>
#include <unordered_set>
#include "PassManager.hpp"
#include "Module.h"
#include "Function.h"
#include "BasicBlock.h"

class LoopInvHoist : public Pass {
public:
    LoopInvHoist(Module *m) : Pass(m) {}
    void run();
    void f1(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* bb,bool print);
    void f2(std::unordered_set<BBset_t *>::iterator bbs,bool print);
    void f3(std::unordered_set<BBset_t *>::iterator bbs,bool print);
    void f4(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* bb,bool print);
    void moveinstr(Instruction* instr,BasicBlock* bb);
    std::unordered_map<BasicBlock*,bool> bbvisit;
    std::unordered_map<BasicBlock*,std::unordered_map<BasicBlock*,bool>> bbformer;
    std::unordered_map<Instruction*,bool> leftnouse;
    std::unordered_map<Instruction*,bool> rightnoassign;
    std::vector<Instruction*> rmlist;
};
