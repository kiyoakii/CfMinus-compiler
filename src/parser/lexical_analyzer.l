%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"

int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}
%}

ws      [" "\t\r\a]+
letter  [A-Za-z]
digit   [0-9]
id      {letter}+
integer {digit}+
float   {digit}+\.|{digit}*\.{digit}+
%x  comment

%%

else    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return ELSE;    }
if      {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return IF;      }
int     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return INT;     }
return  {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return RETURN;  }
void    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return VOID;    }
while   {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return WHILE;   }
float   {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return FLOAT;   }

"+"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return ADD; }
"-"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return SUB; }
"*"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return MUL; }
"/"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return DIV; }
"<"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return LT;  }
"<="    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return LTE; }
">"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return GT;  }
">="    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return GTE; }
"=="    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return EQ;  }
"!="    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return NEQ; }
"="     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return ASSIN;}

";"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return SEMICOLON;   }
","     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return COMMA;       }
"("     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return LPARENTHESE; }
")"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return RPARENTHESE; }
"["     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return LBRACKET;    }
"]"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return RBRACKET;    }
"[]"    {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return ARRAY;       }
"{"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return LBRACE;      }
"}"     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return RBRACE;      }

"/*"            {BEGIN(comment);    pos_end += 2;   }
<comment>\n     {pos_start = pos_end = 1;   ++lines;}
<comment>"*/"   {BEGIN(INITIAL);    pos_end += 2;   }
<comment>.      {++pos_end;}

{ws}        {pos_start = pos_end;  pos_end += yyleng;   return BLANK;       }
{integer}   {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return INTEGER;     }
{float}     {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return FLOATPOINT;  }
{id}        {pos_start = pos_end;  pos_end += yyleng;   pass_node(yytext);  return IDENTIFIER;  }
\n          {pos_start = pos_end = 1;   ++lines;}
. {return ERROR;}


%%