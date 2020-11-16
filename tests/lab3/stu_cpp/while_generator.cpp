#include "BasicBlock.h"
#include "Constant.h"
#include "Function.h"
#include "IRBuilder.h"
#include "Module.h"
#include "Type.h"

#include <iostream>
#include <memory>

#ifdef DEBUG  // 用于调试信息,大家可以在编译过程中通过" -DDEBUG"来开启这一选项
#define DEBUG_OUTPUT std::cout << __LINE__ << std::endl;  // 输出行号的简单示例
#else
#define DEBUG_OUTPUT
#endif

#define CONST_INT(num) \
    ConstantInt::get(num, module)

#define CONST_FP(num) \
    ConstantFP::get(num, module) // 得到常数值的表示,方便后面多次用到

int main() {
    auto module = new Module("while");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    auto aAlloca = builder->create_alloca(Int32Type);
    auto iAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(10), aAlloca);  // a = 10;
    builder->create_store(CONST_INT(0), iAlloca);   // i = 0;
    
    auto conditionBB = BasicBlock::create(module, "condition", mainFun);
    auto trueBB = BasicBlock::create(module, "true", mainFun);
    auto retBB = BasicBlock::create(module, "ret", mainFun);
    builder->create_br(conditionBB);

    // while(i < 10)
    builder->set_insert_point(conditionBB);
    auto iLoad = builder->create_load(iAlloca);
    auto cmp = builder->create_icmp_lt(iLoad, CONST_INT(10));
    builder->create_cond_br(cmp, trueBB, retBB);

    builder->set_insert_point(trueBB);
    // i = i + 1;
    iLoad = builder->create_load(iAlloca);
    builder->create_store(builder->create_iadd(iLoad, CONST_INT(1)), iAlloca);
    // a = a + i;
    auto aLoad = builder->create_load(aAlloca);
    iLoad = builder->create_load(iAlloca);
    builder->create_store(builder->create_iadd(aLoad, iLoad), aAlloca);
    builder->create_br(conditionBB);

    // return a;
    builder->set_insert_point(retBB);
    aLoad = builder->create_load(aAlloca);
    builder->create_ret(aLoad);
    std::cout << module->print();
    delete module;

    return 0;
}
