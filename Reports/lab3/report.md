# lab3 实验报告
PB17111572 李缙

## 问题1: cpp与.ll的对应
### assign

只需要一个 entry BB

```cpp
int main() {
    auto module = new Module("assign");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    // int a[10]
    auto *arrayType = ArrayType::get(Int32Type, 10);
    auto a = builder->create_alloca(arrayType);

    // a[0] = 10
    auto a0GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(0)});
    builder->create_store(CONST_INT(10), a0GEP);

    // a[1] = a[0] * 2
    auto a0Load = builder->create_load(a0GEP);
    auto a1GEP = builder->create_gep(a, {CONST_INT(0), CONST_INT(1)});
    auto mul = builder->create_imul(a0Load, CONST_INT(2));
    builder->create_store(mul, a1GEP);

    // return a[1]
    auto a1Load = builder->create_load(a1GEP);
    builder->create_ret(a1Load);
    std::cout << module->print();
    delete module;
    
    return 0;
}
```

```
define dso_local i32 @main() #0 {
    %1 = alloca i32, align 4
    store i32 0, i32* %1, align 4
    %2 = alloca [10 x i32], align 4 ; int a[10]
    
    ; a[0] = 10
    %3 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 0
    store i32 10, i32* %3, align 4

    ; a[1] = a[0] * 2
    %4 = getelementptr inbounds [10 x i32], [10 x i32]* %2, i64 0, i64 1
    %5 = load i32, i32* %3, align 4
    %6 = mul nsw i32 %5, 2

    ; return a[1]
    store i32 %6, i32* %4, align 4
    %7 = load i32, i32* %4, align 4
    ret i32 %7
}
```

### if

trueBB 对应 ll 中的 %5，falseBB 对应 ll 中的 %6

```cpp
int main() {
    auto module = new Module("if");
    auto builder = new IRBuilder(nullptr, module);
    Type *Int32Type = Type::get_int32_type(module);
    Type *FloatType = Type::get_float_type(module);

    auto mainFun = Function::create(FunctionType::get(Int32Type, {}), "main", module);
    auto bb = BasicBlock::create(module, "entry", mainFun);
    builder->set_insert_point(bb);
    auto retAlloca = builder->create_alloca(Int32Type);
    builder->create_store(CONST_INT(0), retAlloca);

    // float a = 5.555;
    auto aAlloca = builder->create_alloca(FloatType);
    builder->create_store(CONST_FP(5.55), aAlloca);
    auto aLoad = builder->create_load(aAlloca);
    auto fcmp = builder->create_fcmp_gt(aLoad, CONST_FP(1.0));
    auto trueBB = BasicBlock::create(module, "trueBB", mainFun);
    auto falseBB = BasicBlock::create(module, "falseBB", mainFun);
    builder->create_cond_br(fcmp, trueBB, falseBB);
    builder->set_insert_point(trueBB);

    // if(a > 1)
    builder->create_ret(CONST_INT(233));
    
    // return 0;
    builder->set_insert_point(falseBB);
    builder->create_ret(CONST_INT(0));
    std::cout << module->print();
    delete module;

    return 0;
}
```

```
define dso_local i32 @main() #0 {
    %1 = alloca i32, align 4

    ; float a = 5.555;
    %2 = alloca float, align 4
    store i32 0, i32* %1, align 4
    store float 0x40163851E0000000, float* %2, align 4

    ; if(a > 1)
    %3 = load float, float* %2, align 4
    %4 = fcmp ugt float %3, 1.0
    br i1 %4, label %5, label %6
5:
    ret i32 233
6:
    ret i32 0
}
```

### fun

俩函数，俩 BB

```cpp
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
```

```
define dso_local i32 @callee(i32 %0) #0 {
    ; a
    %2 = alloca i32, align 4
    store i32 %0, i32* %2, align 4

    ; return 2 * a
    %3 = load i32, i32* %2, align 4
    %4 = mul nsw i32 2, %3
    ret i32 %4
}

define dso_local i32 @main() #0 {
    ; return callee(110)
    %1 = call i32 @callee(i32 110)
    ret i32 %1
}
```

### while

condition 对应 ll 中的 %4，trueBB 对应 ll 中的 %7，falseBB 对应 ll 中的 %13

```cpp
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
```

```
; ModuleID = 'assign.c'
source_filename = "assign.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

define dso_local i32 @main() #0 {
    %1 = alloca i32, align 4
    %2 = alloca i32, align 4 ; int a
    %3 = alloca i32, align 4 ; int i
    store i32 0, i32* %1, align 4
    store i32 10, i32* %2, align 4 ; a = 10
    store i32 0, i32* %3, align 4 ; i = 0
    br label %4
4:
    ; while(i < 10)
    %5 = load i32, i32* %3, align 4
    %6 = icmp slt i32 %5, 10
    br i1 %6, label %7, label %13
7:
    ; i = i + 1;
    %8 = load i32, i32* %3, align 4
    %9 = add nsw i32 %8, 1
    store i32 %9, i32* %3, align 4

    ; a = a + i;
    %10 = load i32, i32* %2, align 4
    %11 = load i32, i32* %3, align 4
    %12 = add nsw i32 %10, %11
    store i32 %12, i32* %2, align 4
    br label %4
13:
    ; return a
    %14 = load i32, i32* %2, align 4
    ret i32 %14
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
```

## 问题2: Visitor Pattern
exprRoot->numberF->exprE->exprD->numberB->numberA->expC->numberA->numberB

## 问题3: getelementptr
请给出`IR.md`中提到的两种getelementptr用法的区别,并稍加解释:
  - `%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0` 
  - `%2 = getelementptr i32, i32* %1 i32 %0` 

第一种 getelementptr 是先计算数组首地址再算偏移量 %0，第二种是直接用 %1 来算。

## 实验难点
无

## 实验反馈
吐槽?建议?

无