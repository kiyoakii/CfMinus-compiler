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

else    {pos_start = pos_end;  pos_end += yyleng;   return ELSE;    }
if      {pos_start = pos_end;  pos_end += yyleng;   return IF;      }
int     {pos_start = pos_end;  pos_end += yyleng;   return INT;     }
return  {pos_start = pos_end;  pos_end += yyleng;   return RETURN;  }
void    {pos_start = pos_end;  pos_end += yyleng;   return VOID;    }
while   {pos_start = pos_end;  pos_end += yyleng;   return WHILE;   }
float   {pos_start = pos_end;  pos_end += yyleng;   return FLOAT;   }

"+"     {pos_start = pos_end;  pos_end += yyleng;   return ADD; }
"-"     {pos_start = pos_end;  pos_end += yyleng;   return SUB; }
"*"     {pos_start = pos_end;  pos_end += yyleng;   return MUL; }
"/"     {pos_start = pos_end;  pos_end += yyleng;   return DIV; }
"<"     {pos_start = pos_end;  pos_end += yyleng;   return LT;  }
"<="    {pos_start = pos_end;  pos_end += yyleng;   return LTE; }
">"     {pos_start = pos_end;  pos_end += yyleng;   return GT;  }
">="    {pos_start = pos_end;  pos_end += yyleng;   return GTE; }
"=="    {pos_start = pos_end;  pos_end += yyleng;   return EQ;  }
"!="    {pos_start = pos_end;  pos_end += yyleng;   return NEQ; }
"="     {pos_start = pos_end;  pos_end += yyleng;   return ASSIN;}

";"     {pos_start = pos_end;  pos_end += yyleng;   return SEMICOLON;   }
","     {pos_start = pos_end;  pos_end += yyleng;   return COMMA;       }
"("     {pos_start = pos_end;  pos_end += yyleng;   return LPARENTHESE; }
")"     {pos_start = pos_end;  pos_end += yyleng;   return RPARENTHESE; }
"["     {pos_start = pos_end;  pos_end += yyleng;   return LBRACKET;    }
"]"     {pos_start = pos_end;  pos_end += yyleng;   return RBRACKET;    }
"[]"    {pos_start = pos_end;  pos_end += yyleng;   return ARRAY;       }
"{"     {pos_start = pos_end;  pos_end += yyleng;   return LBRACE;      }
"}"     {pos_start = pos_end;  pos_end += yyleng;   return RBRACE;      }

"/*"            {BEGIN(comment);    pos_end += 2;   }
<comment>\n     {pos_start = pos_end = 1;   ++lines;}
<comment>"*/"   {BEGIN(INITIAL);    pos_end += 2;   }
<comment>.      {++pos_end;}

{ws}        {pos_start = pos_end;  pos_end += yyleng;   return BLANK;       }
{integer}   {pos_start = pos_end;  pos_end += yyleng;   return INTEGER;     }
{float}     {pos_start = pos_end;  pos_end += yyleng;   return FLOATPOINT;  }
{id}        {pos_start = pos_end;  pos_end += yyleng;   return IDENTIFIER;  }
\n          {pos_start = pos_end = 1;   ++lines;}
. {return ERROR;}


%%