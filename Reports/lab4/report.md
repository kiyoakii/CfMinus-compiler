# Lab4 实验报告

李缙 PB17111572， 王经文 PB18111722

## 实验要求

使用 LightIR，自动从 AST 生成 ll 代码。

## 实验难点

1. 类型转换

   其实 float 和 int 间的互转还好，主要是在 int1 和 int32 间的互转花费了不少时间。

2. BB 的创建与跳转逻辑

   这里是本次实验的最大难点，在用自己的例子测试中，我们经常发现由于源码的嵌套语句，使得生成出来的 bb 为空/bb 有两个 termination，我们花费了相当多的时间解决对应的问题。

## 实验设计

根据对 CMINUSF 由上至下的分析，得到以下伪码与结构。

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
global_v = create_store(v, global_v)
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
    gep = create_gep(scope.find(id), {ZERO, index})
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

其中，`global_v` 用来传递下层的值到上层，`global_p`用来传递下层的指针到上层。为什么需要两个？主要是因为 AsignExpression 的 visit 函数内需要有左值的指针。而且我们发现，如果是从 AssignExpression 下到 Var 的，那么就无需 load，只需要向上传递 `global_p`，其实可以定义一个 `need_load` 变量，减少 IR 中的 load 冗余，但是时间有限，我们就没有实现。

`name_count` 用来避免一个函数内的 BasicBlock 命名冲突，每次将 name_count 添加在 bb 的名字后面。


### 实验总结

本次实验对于编译器生成 ll 代码的过程有了非常深入的了解，在编写代码中更体会到了编译器的实现难点与设计难点。
