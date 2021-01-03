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
    void find_right_no_assign(std::unordered_set<BBset_t *>::iterator bbs);
    void move_all_instr(std::unordered_set<BBset_t *>::iterator bbs,BasicBlock* base);
    void moveinstr(Instruction* instr,BasicBlock* bb);
    void retmovfinal(BasicBlock* bb);
    std::unordered_map<Instruction*,bool> rightnoassign;
    std::vector<Instruction*> rmlist;
    bool change;
};
