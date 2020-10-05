%option noyywrap
%{

#include <stdio.h>
#include <stdlib.h>

#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

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
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token_stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

    while(token = yylex()){
        switch(token){
            case COMMENT:
                //STUDENT TO DO
            case BLANK:
                //STUDENT TO DO
                break;
            case EOL:
                //STUDENT TO DO
                break;
            case ERROR:
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                token_stream[index].pos_end = pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}



/****************C代码 end*************/
