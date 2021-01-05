#ifndef ACTIVEVARS_HPP
#define ACTIVEVARS_HPP
#include "PassManager.hpp"
#include "Constant.h"
#include "Instruction.h"
#include "Module.h"

#include "Value.h"
#include "IRBuilder.h"
#include <vector>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <queue>
#include <fstream>
#include <algorithm>

class ActiveVars : public Pass
{
public:
    ActiveVars(Module *m) : Pass(m) {}
    void run();
    std::string print();
private:
    Function *func_;
    std::map<BasicBlock *, std::unordered_set<Value *>> live_in, live_out;
    std::map<BasicBlock *, std::unordered_set<Value *>> def, use;
    std::unordered_map<BasicBlock *, bool> visited;
    std::vector<BasicBlock *> DFSList;

    void buildDFSList(Function *func);
    void DFSvisit(BasicBlock* bb);
};

#endif