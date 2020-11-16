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
    auto module = new Module("fun");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    auto calleeFun = Function::create(FunctionType::get(Int32Type, {Int32Type}), "callee", module);
    auto bb = BasicBlock::create(module, "entry", calleeFun);

    builder->set_insert_point(bb);
    auto aAlloca = builder->create_alloca(Int32Type);

    std::vector<Value *> args;  // 获取形参,通过Function中的iterator
    for (auto arg = calleeFun->arg_begin(); arg != calleeFun->arg_end(); arg++) {
        args.push_back(*arg);
    }

    builder->create_store(args[0], aAlloca);

    // return a * 2;
    auto aLoad = builder->create_load(aAlloca);
    builder->create_ret(builder->create_imul(aLoad, CONST_INT(2)));

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);

    // return callee(110);
    builder->create_ret(builder->create_call(calleeFun, {CONST_INT(110)}));
    std::cout << module->print();
    delete module;
    
    return 0;
}
