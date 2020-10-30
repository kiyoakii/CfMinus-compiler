%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "syntax_tree.h"

// external functions from lex
extern int yylex();
extern FILE* yyin;

// external variables from lexical_analyzer module
extern int lines;
extern char *yytext;
extern int pos_end;
extern int pos_start;

// Global syntax tree
syntax_tree *gt;

// Error reporting
void yyerror(const char *s);

// Helper functions written for you with love
syntax_tree_node *node(const char *node_name, int children_num, ...);
%}

%union {
    syntax_tree_node* node;
}

%type <node> program type-specifier relop addop mulop simple-expression declaration-list declaration var-declaration fun-declaration local-declarations compound-stmt statement-list statement expression-stmt iteration-stmt selection-stmt return-stmt expression var additive-expression term factor integer float call params param-list param args arg-list
%token <node> ADD SUB MUL DIV LT LTE GT GTE EQ NEQ ASSIN SEMICOLON LBRACKET RBRACKET COMMA LPARENTHESE RPARENTHESE LBRACE RBRACE IDENTIFIER INTEGER FLOAT ARRAY LETTER ERROR INT VOID IF ELSE WHILE RETURN FLOATPOINT BLANK

%start program

%%

program
: declaration-list { 
    $$ = node("program", 1, $1); gt->root = $$; 
};

declaration-list
: declaration-list declaration { $$ = node("declaration-list", 2, $1, $2); gt->root = $$; }
| declaration { $$ = node("declaration-list", 1, $1); gt->root = $$; };

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

type-specifier  
: INT {
    $$ = node("type-specifier", 1, $1); gt->root = $$;
}
| FLOAT{
    $$ = node("type-specifier", 1, $1); gt->root = $$;
}
| VOID{
    $$ = node("type-specifier", 1, $1); gt->root = $$;
};

fun-declaration 
: type-specifier IDENTIFIER LPARENTHESE params RPARENTHESE compound-stmt {
    $$ = node("fun-declaration", 6, $1, $2, $3, $4, $5, $6); gt->root = $$;
};

params  
: param-list{
    $$ = node("params", 1, $1); gt->root = $$;
}
| VOID{
    $$ = node("params", 1, $1); gt->root = $$;
};

param-list  
: param-list COMMA param {
    $$ = node("param-list", 3, $1, $2, $3); gt->root = $$;
}
| param {
    $$ = node("param-list", 1, $1); gt->root = $$;
};

param   
: type-specifier IDENTIFIER {
    $$ = node("param", 2, $1, $2); gt->root = $$;
}
| type-specifier IDENTIFIER ARRAY {
    $$ = node("param", 3, $1, $2, $3); gt->root = $$;
};

compound-stmt 
: LBRACE local-declarations statement-list RBRACE {
    $$ = node("compound-stmt", 4, $1, $2, $3, $4); gt->root = $$;
};

local-declarations  
: local-declarations var-declaration {
    $$ = node("local-declarations", 2, $1, $2); gt->root = $$;
}
| %empty {
    $$ = node("local-declarations", 0); gt->root = $$;
};

statement-list  
: statement-list statement {
    $$ = node("statement-list", 2, $1, $2); gt->root = $$;
}
| %empty {
    $$ = node("statement-list", 0); gt->root = $$;
};

statement   
: expression-stmt {
    $$ = node("statement", 1, $1); gt->root = $$;
}
| compound-stmt {
    $$ = node("statement", 1, $1); gt->root = $$;
}
| selection-stmt {
    $$ = node("statement", 1, $1); gt->root = $$;
}
| iteration-stmt {
    $$ = node("statement", 1, $1); gt->root = $$;
}
| return-stmt {
    $$ = node("statement", 1, $1); gt->root = $$;
};

expression-stmt 
: expression SEMICOLON {
    $$ = node("expression-stmt", 2, $1, $2); gt->root = $$;
}
| SEMICOLON{
    $$ = node("expression-stmt", 1, $1); gt->root = $$;
};

selection-stmt 
: IF LPARENTHESE expression RPARENTHESE statement {
    $$ = node("selection-stmt", 5, $1, $2, $3, $4, $5); gt->root = $$;
}
| IF LPARENTHESE expression RPARENTHESE statement ELSE statement {
    $$ = node("selection-stmt", 7, $1, $2, $3, $4, $5, $6, $7); gt->root = $$;
};

iteration-stmt 
: WHILE LPARENTHESE expression RPARENTHESE statement {
    $$ = node("iteration-stmt", 5, $1, $2, $3, $4, $5); gt->root = $$;
};

return-stmt 
: RETURN SEMICOLON {
    $$ = node("return-stmt", 2, $1, $2); gt->root = $$;
}
| RETURN expression SEMICOLON {
    $$ = node("return-stmt", 3, $1, $2, $3); gt->root = $$;
};

expression 
: var ASSIN expression {
    $$ = node("expression",3, $1, $2, $3); gt->root = $$;
}
| simple-expression {
    $$ = node("expression",1, $1); gt->root = $$;
};

var 
: IDENTIFIER {
    $$ = node("var",1, $1); gt->root = $$;
}
| IDENTIFIER LBRACKET expression RBRACKET {
    $$ = node("var", 4, $1, $2, $3, $4); gt->root = $$;
};

simple-expression
: additive-expression relop additive-expression {
    $$ = node("simple-expression", 3, $1, $2, $3); gt->root = $$;
}
| additive-expression {
    $$ = node("simple-expression",1, $1); gt->root = $$;
};

relop 
: LT {
    $$ = node("relop",1, $1); gt->root = $$;
}
| LTE {
    $$ = node("relop",1, $1); gt->root = $$;
}
| GT {
    $$ = node("relop",1, $1); gt->root = $$;
}
| GTE {
    $$ = node("relop",1, $1); gt->root = $$;
}
| EQ {
    $$ = node("relop",1, $1); gt->root = $$;
}
| NEQ {
    $$ = node("relop",1, $1); gt->root = $$;
};

additive-expression 
: additive-expression addop term {
    $$ = node("additive-expression",3, $1, $2, $3); gt->root = $$;
}
| term {
    $$ = node("additive-expression",1, $1); gt->root = $$;
};

addop 
: ADD {
    $$ = node("addop",1, $1); gt->root = $$;
}
| SUB {
    $$ = node("addop",1, $1); gt->root = $$;
};

term 
: term mulop factor {
    $$ = node("term",3, $1, $2, $3); gt->root = $$;
}
| factor {
    $$ = node("term",1, $1); gt->root = $$;
};

mulop 
: MUL {
    $$ = node("mulop",1, $1); gt->root = $$;
}| DIV {
    $$ = node("mulop",1, $1); gt->root = $$;
};

factor 
: LPARENTHESE expression RPARENTHESE  {
    $$ = node("factor",3, $1, $2, $3); gt->root = $$;
}
| var {
    $$ = node("factor",1, $1); gt->root = $$;
}
| call {
    $$ = node("factor",1, $1); gt->root = $$;
}
| integer {
    $$ = node("factor",1, $1); gt->root = $$;
}
| float{
    $$ = node("factor",1, $1); gt->root = $$;
}

integer 
: INTEGER {
    $$ = node("integer",1, $1); gt->root = $$;
};

float 
: FLOATPOINT {
    $$ = node("float",1, $1); gt->root = $$;
};

call 
: IDENTIFIER LPARENTHESE args RPARENTHESE {
    $$ = node("call",4, $1, $2, $3, $4); gt->root = $$;
};

args 
: arg-list {
    $$ = node("args",1, $1); gt->root = $$;
}
| %empty {
    $$ = node("args", 0); gt->root = $$;
};

arg-list 
: arg-list COMMA expression {
    $$ = node("arg-list",3, $1, $2, $3); gt->root = $$;
}
| expression {
    $$ = node("arg-list",1, $1); gt->root = $$;
};

%%

/// The error reporting function.
void yyerror(const char *s)
{
    // TO STUDENTS: This is just an example.
    // You can customize it as you like.
    fprintf(stderr, "error at line %d column %d: %s\n", lines, pos_start, s);
}

/// Parse input from file `input_path`, and prints the parsing results
/// to stdout.  If input_path is NULL, read from stdin.
///
/// This function initializes essential states before running yyparse().
syntax_tree *parse(const char *input_path)
{
    if (input_path != NULL) {
        if (!(yyin = fopen(input_path, "r"))) {
            fprintf(stderr, "[ERR] Open input file %s failed.\n", input_path);
            exit(1);
        }
    } else {
        yyin = stdin;
    }

    lines = pos_start = pos_end = 1;
    gt = new_syntax_tree();
    yyrestart(yyin);
    yyparse();
    return gt;
}

/// A helper function to quickly construct a tree node.
///
/// e.g.
///     $$ = node("program", 1, $1);
///     $$ = node("local-declarations", 0);
syntax_tree_node *node(const char *name, int children_num, ...)
{
    syntax_tree_node *p = new_syntax_tree_node(name);
    syntax_tree_node *child;
    if (children_num == 0) {
        child = new_syntax_tree_node("epsilon");
        syntax_tree_add_child(p, child);
    } else {
        va_list ap;
        va_start(ap, children_num);
        for (int i = 0; i < children_num; ++i) {
            child = va_arg(ap, syntax_tree_node *);
            syntax_tree_add_child(p, child);
        }
        va_end(ap);
    }
    return p;
}
