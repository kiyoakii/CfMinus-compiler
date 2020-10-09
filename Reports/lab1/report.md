# lab1实验报告
PB17111572 李缙
## 实验要求

本次实验需要根据`cminux-f`的词法补全[lexical_analyer.l](./src/lexer/lexical_analyzer.l)文件，完成词法分析器，能够输出识别出的`token`，`type` ,`line(刚出现的行数)`，`pos_start(该行开始位置)`，`pos_end(结束的位置,不包含)`。如：

文本输入：

```c
 int a;
```

则识别结果应为：

```shell
int     280     1       2       5
a       285     1       6       7
;       270     1       7       8
```

## 实验难点

1. 理解 FLex 结构和用法。
2. 注释的检测语法（Flex 的 Start Condition）。

## 实验设计

### 声明与定义

先声明一些常用变量：

```
ws      [" "\t\r\a]+
letter  [A-Za-z]
digit   [0-9]
id      {letter}+
integer {digit}+
float   {digit}+\.|{digit}*\.{digit}+
```

然后声明一个 Start Condition 标识符：

```
%x  comment
```

comment 的使用见下节。

### 模式定义

根据头文件的预定义，先写出一些固定的标识符的模式定义。

**关键字(仅给出部分示例)**

```
else {pos_start = pos_end; pos_end += yyleng; return ELSE;}
if   {pos_start = pos_end; pos_end += yyleng; return IF;  }
int  {pos_start = pos_end; pos_end += yyleng; return INT; }
```

**专用符号（仅给出部分示例）**

```
"+"  {pos_start = pos_end;  pos_end += yyleng;   return ADD; }
"-"  {pos_start = pos_end;  pos_end += yyleng;   return SUB; }
```

**特殊符号**

```
\n          {pos_start = pos_end = 1;   ++lines;}
```

**注释**

```
"/*"            {BEGIN(comment);    pos_end += 2;   }
<comment>\n     {pos_start = pos_end = 1;   ++lines;}
<comment>"*/"   {BEGIN(INITIAL);    pos_end += 2;   }
<comment>.      {++pos_end;}
```

在上面我们已经确定符号 comment 为注释的 Start Condition 符号，也即 Flex 会在遇到 /* 时将文本记为 BEGIN(comment)，意在进入 comment 状态。与之对应的，BEGIN(INITIAL) 即为退出 comment 状态。

需要注意的是，上述 4 句的顺序是很关键的，因为要保证在注释中不会出现 */ 这个字符串，同时正确对 pos_end 和 lines 等变量计数。

## 实验结果验证
### 样例一



## 实验反馈

本次实验较简单。