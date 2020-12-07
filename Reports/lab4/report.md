# Lab4 实验报告

小组成员 姓名 学号

## 实验要求

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
v = global_v
visit expression
create_store(v, global_v)
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
		r = load global_v
    visit additive_expression
    l = load global_v
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
global_v = create_load(scope.find(id))
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
```



## 实验难点

实验中遇到哪些挑战

## 实验设计



请写明为了顺利完成本次实验，加入了哪些亮点设计，并对这些设计进行解释。
可能的阐述方向有:

1. 如何设计全局变量
2. 遇到的难点以及解决方案
3. 如何降低生成 IR 中的冗余
4. ...


### 实验总结

此次实验有什么收获

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
