# Lab5 实验报告

队长 李缙 PB17111572

队员 王经文 PB18111722

队员 简子哲 PB17000008

## 实验要求

1.常量传播

能计算出结果的变量，就直接替换为常量，删除该句子，并在过程中将该变量都转换为常量。整型和浮点型都要考虑。删除无用分支并将条件跳转转换为强制性跳转。

2.循环不变式外提：将CFG中在一个循环中结果不变的表达式提到循环之前，可以外提的包括8种双目运算和3种格式转换。

3.

## 实验难点

实验中遇到哪些挑战

## 实验设计

* 常量传播实验报告ConstPropagation部分
  
    实现思路：
    
    1.先将常量进行折叠，即遇到整形操作符、浮点数操作符时，用`operand()`函数进行取值计算。（get_operand()得到Value* 型的变量，需要转换一下），再用`replace_all_use_with()`将这句话删除，放入等待删除队列中。
    
    ·遇到整型操作符：
    
    ```C++
    auto binary_instr = dynamic_cast<BinaryInst *>(instr);
                        auto v1 = cast_constantint(instr->get_operand(0));
                        auto v2 = cast_constantint(instr->get_operand(1));
                        if(v1 != nullptr && v2 != nullptr){
                            auto flod_const = constFolder->compute(binary_instr->get_instr_type(), v1, v2);
                            instr->replace_all_use_with(flod_const);
                            wait_delete.push_back(instr);
                        }
    ```
    
    ·遇到浮点型操作符
    
    ```c++
    auto binary_instr = dynamic_cast<BinaryInst *>(instr);
                        auto v1 = cast_constantfp(instr->get_operand(0));
                        auto v2 = cast_constantfp(instr->get_operand(1));
                        if(v1 != nullptr && v2 != nullptr){
                            auto flod_const = constFolder->compute(binary_instr->get_instr_type(), v1, v2);
                            instr->replace_all_use_with(flod_const);
                            wait_delete.push_back(instr);
                        }
    ```
    
    ·遇到整型比较操作符
    
    ```c++
    auto cmp_instr = dynamic_cast<CmpInst *>(instr);
    auto v1 = cast_constantint(instr->get_operand(0));
    auto v2 = cast_constantint(instr->get_operand(1));
    if(v1 != nullptr && v2 != nullptr){
        auto flod_const = constFolder->compute(cmp_instr->get_cmp_op(), v1, v2);
        instr->replace_all_use_with(flod_const);
        wait_delete.push_back(instr);
    }
    ```
    
    ·遇到浮点型比较操作符
    
    ```c++
    auto cmp_instr = dynamic_cast<FCmpInst *>(instr);
    auto v1 = cast_constantfp(instr->get_operand(0));
    auto v2 = cast_constantfp(instr->get_operand(1));
    if(v1 != nullptr && v2 != nullptr){
        auto flod_const = constFolder->compute(cmp_instr->get_cmp_op(), v1, v2);
        instr->replace_all_use_with(flod_const);
        wait_delete.push_back(instr);
    }
    ```
    
    2.遇到需要整型变浮点型、浮点型变整型、bool型变整型的变量，需要类型转换。和第一部分一样，全部替换，然后放入待删除队列。
    
    ```c++
    ConstantFP *val =cast_constantfp(instr->get_operand(0));
    if(val != nullptr){
        instr->replace_all_use_with(ConstantInt::get((int) val->get_value(),m_));
        wait_delete.push_back(instr);
    }
    ```
    
    ```c++
    ConstantInt *val =cast_constantint(instr->get_operand(0));
    if(val != nullptr){
        instr->replace_all_use_with(ConstantFP::get((float) val->get_value(),m_));
        wait_delete.push_back(instr);
    }
    ```
    
    ```c++
    ConstantInt *val =cast_constantint(instr->get_operand(0));
    if(val != nullptr){
        instr->replace_all_use_with(ConstantInt::get((int) val->get_value(),m_));
        wait_delete.push_back(instr);
    }
    ```
    
    3.if-else跳转问题，找到终结语句，获取`br`，将`br`转换为整型并赋值给`cond`，当cond为常数时，进行判断：`cond`为零时，强制跳转到第二块（falsebb），并将终结语句放入待删除队列；`cond`非零时，强制跳转到第一块（truebb），并将终结语句放入待删除队列。
    
    ```c++
    auto br = bb->get_terminator();
    if(dynamic_cast<BranchInst *>(br)->is_cond_br()){
        auto cond = dynamic_cast<ConstantInt *>(br->get_operand(0));
        auto truebb = dynamic_cast<BasicBlock*>(br->get_operand(1));
        auto falsebb = dynamic_cast<BasicBlock*>(br->get_operand(2));
        if(cond){
            if(cond->get_value() == 0){
                BranchInst::create_br(falsebb,bb);
                wait_delete.push_back(instr);
            }
            else{
                BranchInst::create_br(truebb,bb);
                wait_delete.push_back(instr);
                }
        }
    }
    ```
    
    4.最后将队列里的东西全部删除
    
    ```c++
    for ( auto instr : wait_delete)
    {
        bb->delete_instr(instr);
    }
    ```
    
    优化前后的IR对比（举一个例子）并辅以简单说明：用`testcase-1`举例
    
    ```c
    void main(void){
        int i;
        int idx;
    
        i = 0;
        idx = 0;
    
        while(i < 100000000)
        {
            idx = 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 + 1 ;
            i=i+idx*idx*idx*idx*idx*idx*idx*idx/(idx*idx*idx*idx*idx*idx*idx*idx);
        }
    	output(idx*idx);
        return ;
    }
    ```
    
    优化前：
    
    ```bash
    define void @main() {
    label_entry:
      br label %label2
    label2:                                                ; preds = %label_entry, %label7
      %op78 = phi i32 [ 0, %label_entry ], [ %op73, %label7 ]
      %op79 = phi i32 [ 0, %label_entry ], [ %op40, %label7 ]
      %op4 = icmp slt i32 %op78, 100000000
      %op5 = zext i1 %op4 to i32
      %op6 = icmp ne i32 %op5, 0
      br i1 %op6, label %label7, label %label74
    label7:                                                ; preds = %label2
      %op8 = add i32 1, 1
      %op9 = add i32 %op8, 1
      %op10 = add i32 %op9, 1
      %op11 = add i32 %op10, 1
      %op12 = add i32 %op11, 1
      %op13 = add i32 %op12, 1
      %op14 = add i32 %op13, 1
      %op15 = add i32 %op14, 1
      %op16 = add i32 %op15, 1
      %op17 = add i32 %op16, 1
      %op18 = add i32 %op17, 1
      %op19 = add i32 %op18, 1
      %op20 = add i32 %op19, 1
      %op21 = add i32 %op20, 1
      %op22 = add i32 %op21, 1
      %op23 = add i32 %op22, 1
      %op24 = add i32 %op23, 1
      %op25 = add i32 %op24, 1
      %op26 = add i32 %op25, 1
      %op27 = add i32 %op26, 1
      %op28 = add i32 %op27, 1
      %op29 = add i32 %op28, 1
      %op30 = add i32 %op29, 1
      %op31 = add i32 %op30, 1
      %op32 = add i32 %op31, 1
      %op33 = add i32 %op32, 1
      %op34 = add i32 %op33, 1
      %op35 = add i32 %op34, 1
      %op36 = add i32 %op35, 1
      %op37 = add i32 %op36, 1
      %op38 = add i32 %op37, 1
      %op39 = add i32 %op38, 1
      %op40 = add i32 %op39, 1
      %op44 = mul i32 %op40, %op40
      %op46 = mul i32 %op44, %op40
      %op48 = mul i32 %op46, %op40
      %op50 = mul i32 %op48, %op40
      %op52 = mul i32 %op50, %op40
      %op54 = mul i32 %op52, %op40
      %op56 = mul i32 %op54, %op40
      %op59 = mul i32 %op40, %op40
      %op61 = mul i32 %op59, %op40
      %op63 = mul i32 %op61, %op40
      %op65 = mul i32 %op63, %op40
      %op67 = mul i32 %op65, %op40
      %op69 = mul i32 %op67, %op40
      %op71 = mul i32 %op69, %op40
      %op72 = sdiv i32 %op56, %op71
      %op73 = add i32 %op78, %op72
      br label %label2
    label74:                                                ; preds = %label2
      %op77 = mul i32 %op79, %op79
      call void @output(i32 %op77)
      ret void
    }
    ```
    
    优化后：
    
    ```bash
    define void @main() {
    label_entry:
      br label %label2
    label2:                                                ; preds = %label_entry, %label7
      %op78 = phi i32 [ 0, %label_entry ], [ %op73, %label7 ]
      %op79 = phi i32 [ 0, %label_entry ], [ 34, %label7 ]
      %op4 = icmp slt i32 %op78, 100000000
      %op5 = zext i1 %op4 to i32
      %op6 = icmp ne i32 %op5, 0
      br i1 %op6, label %label7, label %label74
    label7:                                                ; preds = %label2
      %op73 = add i32 %op78, 1
      br label %label2
    label74:                                                ; preds = %label2
      %op77 = mul i32 %op79, %op79
      call void @output(i32 %op77)
      ret void
    }
    ```
    
    很显然，将常量结果计算出来代入，`idx = 34`，`i = i + 1`


* 循环不变式外提
    实现思路：
    第一层循环：自内而外遍历所有循环
    第二层循环：不断循环直到不能再做出任何改变
            第一步：查找并标记当前循环内所有右值没有被赋值的表达式（称为“外提表达式”）
            第二步：查找当前循环的入口节点的循环外前驱（称为“目标基本块”）（入口节点有两个前驱，一个循环内，一个循环外）
            第三步：将所有“外提表达式”复制到“目标基本块”的结尾，并将原表达式添加进“删除列表”
                    这里不用考虑顺序：
                            假设语 句b需要语句a的结果，则在外提语句a时，语句b还不属于“外提表达式”
                            等到外提语句b时，语句a必定已经在“目标基本块”里面了，语句b依旧在语句a之后）
            第四步：将“目标基本块”里的终结语句复制到“目标基本块”的结尾，并将原表达式添加进“删除列表”
            第五步：删除所有“删除列表”里的语句。
* 相应代码：
```
cminus：
void main(void)
{
    int i;
    int a;
    i=0;
    a=5;
    while (i<a*a)
        i=i+1;
	output(i);
    return ;
}

第一、二步：
define void @main() {
label_entry:（目标基本块）
  br label %label2
label2:                                                ; preds = %label_entry, %label10
  %op15 = phi i32 [ 0, %label_entry ], [ %op12, %label10 ]
  %op6 = mul i32 5, 5（外提表达式）
  %op7 = icmp slt i32 %op15, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label13
label10:                                                ; preds = %label2
  %op12 = add i32 %op15, 1
  br label %label2
label13:                                                ; preds = %label2
  call void @output(i32 %op15)
  ret void
}

第三步：
define void @main() {
label_entry:（目标基本块）
  br label %label2
  %op6 = mul i32 5, 5（新外提表达式）
label2:                                                ; preds = %label_entry, %label10
  %op15 = phi i32 [ 0, %label_entry ], [ %op12, %label10 ]
  %op6 = mul i32 5, 5（旧外提表达式）
  %op7 = icmp slt i32 %op15, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label13
label10:                                                ; preds = %label2
  %op12 = add i32 %op15, 1
  br label %label2
label13:                                                ; preds = %label2
  call void @output(i32 %op15)
  ret void
}

第四步：
define void @main() {
label_entry:（目标基本块）
  br label %label2（旧终结表达式）
  %op6 = mul i32 5, 5（新外提表达式）
  br label %label2（新终结表达式）
label2:                                                ; preds = %label_entry, %label10
  %op15 = phi i32 [ 0, %label_entry ], [ %op12, %label10 ]
  %op6 = mul i32 5, 5（外提表达式）
  %op7 = icmp slt i32 %op15, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label13
label10:                                                ; preds = %label2
  %op12 = add i32 %op15, 1
  br label %label2
label13:                                                ; preds = %label2
  call void @output(i32 %op15)
  ret void
}

第五步：
define void @main() {
label_entry:（目标基本块）
  %op6 = mul i32 5, 5（新外提表达式）
  br label %label2（新终结表达式）
label2:                                                ; preds = %label_entry, %label10
  %op15 = phi i32 [ 0, %label_entry ], [ %op12, %label10 ]
  %op7 = icmp slt i32 %op15, %op6
  %op8 = zext i1 %op7 to i32
  %op9 = icmp ne i32 %op8, 0
  br i1 %op9, label %label10, label %label13
label10:                                                ; preds = %label2
  %op12 = add i32 %op15, 1
  br label %label2
label13:                                                ; preds = %label2
  call void @output(i32 %op15)
  ret void
}
```
* 活跃变量分析
    实现思路：
    相应的代码：

### 实验总结

认识到了静态单赋值格式的好处，并因此删掉了一半的代码。

### 实验反馈 （可选 不会评分）

对本次实验的建议

### 组间交流 （可选）

本次实验和哪些组（记录组长学号）交流了哪一部分信息
