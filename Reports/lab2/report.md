# lab2 实验报告
PB17111572 李缙
## 实验要求

本次实验需要各位同学首先将自己的 lab1 的词法部分复制到 `/src/parser` 目录的 [lexical_analyzer.l]() 并合理修改相应部分，然后根据 `cminus-f` 的语法补全 [syntax_analyer.y]() 文件，完成语法分析器，要求最终能够输出解析树。

## 实验难点

1. 理解 FLex 结构和用法。
2. 注释的检测语法（Flex 的 Start Condition）。

## 实验设计

### 声明与定义

```c
extern FILE* yyin;
```

```
%union {
    syntax_tree_node* node;
}

%type <node> program type-specifier ...

%token <node> ADD SUB MUL DIV ...

%start program
```

### Rule 定义

根据文档编写。

**(仅给出部分示例)**

```
declaration
: var-declaration { $$ = node("declaration", 1, $1); gt->root = $$; }
| fun-declaration { $$ = node("declaration", 1, $1); gt->root = $$; };

var-declaration
: type-specifier IDENTIFIER SEMICOLON { 
    $$ = node("var-declaration", 3, $1, $2, $3); gt->root = $$; 
}
| type-specifier IDENTIFIER LBRACKET INTEGER RBRACKET SEMICOLON { 
    $$ = node("var-declaration", 6, $1, $2, $3, $4, $5, $6); gt->root = $$; 
};
```

## 实验结果验证

已通过 simple 和 normal 测试。

## 实验反馈

本次实验较简单。