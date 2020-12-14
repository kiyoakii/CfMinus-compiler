# Lab4 实验报告

李缙 PB17111572， 王经文 PB18111722

## 实验要求

使用 LightIR，自动从 AST 生成 ll 代码。

## 实验难点

1. 类型转换

   其实 float 和 int 间的互转还好，主要是在 int1 和 int32 间的互转花费了不少时间。

2. BB 的创建与跳转逻辑

   这里是本次实验的最大难点，在用自己的例子测试中，我们经常发现由于源码的嵌套语句，使得生成出来的 bb 为空/bb 有两个 termination，我们花费了相当多的时间解决对应的问题。
   
3. 对 gep 的处理，主要在 FuncDeclaration 和 Call 内：

   * FuncDeclaration 时要判断形参是否是数组，如果是，形参的类型为指针
   * Call 内要判断实参是否是数组，如果是，创建 gep，传入指针

## 实验设计

根据对 CMINUSF 由上至下的分析，得到以下的基本伪码与结构。当然了，在最初的伪码中，我们直接写了 visit，事实上都是对应元素的accept，请看后面的「一次失败的尝试」处的具体分析。

### Program

包含：declarations

```pseudocode
遍历declarations:
	1. 若是var_decl：
		判断：数组 or 变量，创建全局变量
	2. 若是fun_decl:
		进入fun的visit
```

### FunDeclaration

包含：params 与 compound_stmt

```pseudocode
scope.enter()
for p in params:
    visit p
		l.append(global_v) // l是函数参数列表
Create f using l
create basic block "entry"
builder->set_insert_point
visit compound_stmt
scope.exit()
scope.push(f)
```

### VarDeclaration

包含：type, Num

```pseudocode
if num not nullptr:
    global_v = create arr
else:
    global_v = create num
scope.push(global_v)
```

### Param

包含：type, id

```
create param
global_v = param
```

### Compound_stmt

包含：local_declarations, statement_list

```pseudocode
scope.enter()
for d in local_decl:
    visit d // 注意这里，因为已经有底层函数了，无需任何其它动作
for s in statement_list:
    visit s
```

### Statement

分为五种：

* expression-stmt
* compound_stmt (solved)
* selection_stmt
* iteration_stmt
* return-stmt

### Expression_stmt

包含一个指向expression的指针

```
if expression not nullptr:
    visit expression
```

### Selection_stmt

包含expression, if_statement, else_statement

```
visit expression
cmp = global_v
create TrueBB
create FalseBB
create retBB
create br(cmp, TrueBB, FalseBB)

builder->set_insert(TrueBB)
visit if_statement

if else_statement not nullptr:
		builder->set_insert(FalseBB)
		visit else_statement

builder->set_insert(retBB)
```

### Iteration_stmt

包含：expression, statement

```
create iterBB
create conBB
create retBB

builder->set_insert(conBB)
visit expression
cmp = global_v
create condition_br(cmp, iterBB, retBB)


builder->set_insert(iterBB)
visit statement
create br(conBB)

builder->set_insert(retBB)
```

### return_stmt

包含：expression

```
if expression not nullptr:
		visit expression
		create_ret(global_v)
else:
    create_ret(nullptr)
```

### Expression

不需要自己处理，框架动态分配

### AssignExpression

包含：var, expression

```
visit var
v = scope.find(var) // v need to be a pointer, not loaded value
visit expression
global_v = create_store(v, global_p)
```

### SimpleExpression

包含：additive_expression_l, additive_expression_r, op

```
visit additive_expression_l
if additive_expression_r not nullptr:
    l = load global_v
    visit additive_expression_r
    r = load global_v
    global_v = create_cmp_op(l, r)
```

### AdditiveExpression

包含：additive_expression, term, op

```
visit term
if additive_expression not nullptr:
		r = global_v
    visit additive_expression
    l = global_v
    global_v = create_op(l， r)
```

### Term

包含：term, factor, op

```
visit factor
if term not nullptr:
    r = global_v
    visit term
    global_v = create_op(global_v, r)
```

### Factor

不需要自己处理，框架动态分配

### Var

包含：id, expression

```
if expression not nullptr:
		create TrueBB, FalseBB
    visit expression
    index = global_v
    cmp = create_icmp_lt(index, ZERO)
    create_br(cmp, TrueBB, FalseBB)
    builder->insert_bb(FalseBB)
    create_call(scope.find("neg_idx_error"), {})
    
    builder->insert_bb(TrueBB)
    gep = create_gep // note that it should be different with array and pointer
    global_v = create_load(gep)
    global_p = gep
else:
    global_v = create_load(scope.find(id))
    global_p = scope.find(id)
```

### 如何设计全局变量

全局变量有：

```cpp
Value* global_v;
Value* global_p;
size_t name_count;
```

其中，`global_v` 用来传递下层的值到上层，`global_p`用来传递下层的指针到上层。为什么需要两个？主要是因为有些时候上层需要的是指针而不是 loaded value。主要是体现在两个场景上：

1. AsignExpression 的 visit 函数内需要有左值的指针。而且我们发现，如果是从 AssignExpression 下到 Var 的，那么就无需 load，只需要向上传递 `global_p`，其实可以定义一个 `need_load` 变量，减少 IR 中的 load 冗余，但是时间有限，我们就没有实现。
2. 在 Call 的 visit 函数内，可能需要将数组转为指针，而 ```create_gep``` 中需要有数组的指针。

`name_count` 用来避免一个函数内的 BasicBlock 命名冲突，每次将 name_count 添加在 bb 的名字后面。

### 一次失败的尝试

在最开始的尝试中，由于对 visitor pattern 的不熟悉，我们犯了一个严重的错误。

在 Program 的 visit 函数内，我们分别向下判断了每个 declaration 的类型，甚至我们还直接调用了对应的 visit 函数。而事实上，我们编写的部分是「Real visitor 去访问不同 element 的函数」，从编程范式的角度讲，我们只能做两件事：

1. 对当前 element 进行某些处理
2. 让与当前 element 相关的其它 element 接受访问（这里最重要，所以我们不能直接在当前 element 的 visit 内调用其它 element 的 visit）

顺便附上当时的错误代码（其实还想开个 issue 分享一下这个问题的，但是不知道在 ddl 前是否合适）：

```cpp
void CminusfBuilder::visit(ASTProgram &node) {
 for (auto &decl : node.declarations) {
     auto var_decl = dynamic_cast<ASTVarDeclaration*>(&node);
     if (var_decl) {
         if (var_decl->num == nullptr) {
             // 声明变量
             Value* var_alloca;
             if (var_decl->type == TYPE_INT) {
                 auto int_t = Type::get_int32_type(module.get());
                 auto initializer = ConstantZero::get(int_t, module.get());
                 var_alloca = GlobalVariable::create("global_v" + std::to_string(name_count++), module.get(), int_t, false, initializer);
             } else if (var_decl->type == TYPE_FLOAT) {
                 auto float_t = Type::get_float_type(module.get());
                 auto initializer = ConstantZero::get(float_t, module.get());
                 var_alloca = GlobalVariable::create("global_v" + std::to_string(name_count++), module.get(), float_t, false, initializer);
             }
             scope.push(var_decl->id, var_alloca);
         } else {
             // 声明数组
             Value* arr_alloca;
             if (var_decl->type == TYPE_INT) {
                 auto int_t = Type::get_int32_type(module.get());
                 if (var_decl->num->i_val < 0) {
                     builder->create_call(scope.find("neg_idx_except_fun"), {});
                 }
                 auto arr_t = Type::get_array_type(int_t, var_decl->num->i_val);
                 auto initializer = ConstantZero::get(arr_t, module.get());
                 arr_alloca = GlobalVariable::create("global_arr" + std::to_string(name_count++), module.get(), arr_t, false, initializer);
             } else if (var_decl->type == TYPE_FLOAT) {
                 auto float_t = Type::get_float_type(module.get());
                 auto arr_t = Type::get_array_type(float_t, var_decl->num->i_val);
                 auto initializer = ConstantZero::get(arr_t, module.get());
                 arr_alloca = GlobalVariable::create("global_arr" + std::to_string(name_count++), module.get(), arr_t, false, initializer);
             }
             scope.push(var_decl->id, arr_alloca);
         }
         continue;
     }

     auto fun_decl = dynamic_cast<ASTFunDeclaration*>(&node);
     if (fun_decl) {
         fun_decl->accept(*this);
         continue;
     }
 }
}
```

## 实验测试

用了一些看起来有点好玩的例子：

**testcase 1**

```c
int compare(int a[], int b[]) {
    return a[0] <= b[0];
}

void main(void){
    int a[2];
    int b[2];
    int c;
    int i;
    a[0] = 1;
    b[0] = 1;
    b[1] = 2;
    c = 3;
    i = 0;
    if(a[0]<b[1]){
        if(b[1]<c){
            while(i<c){
                output(i);
                i=i+1;
            }
            output(i);
        } else {
            output(i+1);
        }
    } else {
        output(compare(a, b));
    }
    output(compare(a, b));
    return;
}
```

**testcase 2**

```c
int arr[100];

void quicksort(int start, int len) {
    int pivot;
    int i;
    int j;
    int k;
    int t;
    if (len <= 1) return;
    pivot = arr[start + len - 1];
    i = 0;
    j = 0;
    k = len;
    while (i < k) {
        if (arr[start + i] < pivot) {
            t = arr[start + i];
            arr[start + i] = arr[start + j];
            arr[start + j] = t;
            i = i + 1;
            j = j + 1;
        } else if (pivot < arr[start + i]) {
            k = k - 1;
            t = arr[start + i];
            arr[start + i] = arr[start + k];
            arr[start + k] = t;
        } else
            i = i + 1;
    }
    quicksort(start, j);
    quicksort(start + k, len - k);
    return;
}

void main(void) {
    int n;
    int i;
    i = 0;
    n = input();
    while (i < n) {
        arr[i] = input();
        i = i + 1;
    }
    quicksort(0, n);
    i = 0;
    while (i < n) {
        output(arr[i]);
        i = i + 1;
    }
    return;
}
```

## 实验总结

本次实验对于编译器生成 ll 代码的过程有了非常深入的了解，在编写代码中更体会到了编译器的实现难点与设计难点。当然了，面向测试解决 bug 是非常痛快的。算上助教的 testcase 和自己出的，我们一共测了 35 个源码，当看到归并排序、快速排序之类的程序都能正常运行的时候，成就感真的很强。
