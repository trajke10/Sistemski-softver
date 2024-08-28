%{
  #include "parser.hpp"
  #include <string.h>
  #include "../inc/Asembler.hpp"

%}

%option outfile="./misc/lexer.cpp" header-file="./misc/lexer.hpp"


LPAR "["
RPAR "]"
COMMA ","
EQ "="
IMM "$"
PER "%"
TO ":"
PLUS "+"

SP "%sp"
PC "%pc"
COMMENT "#"[^\n\r]*
DIR "."[a-zA-Z]+
IDENT [a-zA-Z_][a-zA-Z_0-9]*
IMMOPR [0-9]+|0[xX][0-9a-fA-F]+
EOL \n
WS [ \t\r]*
REG ("%r"|"%R")("1"|"2"|"3"|"4"|"5"|"6"|"7"|"8"|"9"|"10"|"12"|"13"|"14"|"15")
CSR "%status"|"%handler"|"%cause"

%% /*pravila*/

{COMMENT} {}

{DIR} {
        yylval.a=new Argument("symb",0,yytext);
        return DIR;}

{IMMOPR} {
        if(yytext[0]!='0'){
                yylval.a=new Argument("num",stol(yytext,0,10),"0");
        }else
                yylval.a=new Argument("num",stol(yytext,0,16),"0");
        return IMMOPR;}

"HALT"|"halt" {return HALT_T;}
"RET"|"ret" {return RET_T;}
"INT"|"int" {return INT_T;}
"IRET"|"iret" {return IRET_T;}
"CALL"|"call" {return CALL_T;}
"JMP"|"jmp" {return JMP_T;}
"BEQ"|"beq" {return BEQ_T;}
"BNE"|"bne" {return BNE_T;}
"BGT"|"bgt" {return BGT_T;}
"PUSH"|"push" {return PUSH_T;}
"POP"|"pop" {return POP_T;}
"XCHG"|"xchg" {return XCHG_T;}
"ADD"|"add" {return ADD_T;}
"SUB"|"sub" {return SUB_T;}
"MUL"|"mul" {return MUL_T;}
"NOT"|"not" {return NOT_T;}
"DIV"|"div" {return DIV_T;}
"AND"|"and" {return AND_T;}
"OR"|"or" {return OR_T;}
"XOR"|"xor" {return XOR_T;}
"SHL"|"shl" {return SHL_T;}
"LD"|"ld" {return LD_T;}
"SHR"|"shr" {return SHR_T;}
"ST"|"st" {return ST_T;}
"CSRRD"|"csrrd" {return CSRRD_T;}
"CSRWR"|"csrwr" {return CSRWR_T;}


{CSR} {int i;
        if(strcmp(yytext,"%status")==0){
                i=0;
        }else if(strcmp(yytext,"%handler")==0){
                i=1;
        }else{
                i=2;
        }
        yylval.a=new Argument("csr",i,yytext);
        return CSR;}

{IDENT} { yylval.a=new Argument("symb",0,yytext); 
          return IDENT;}

{WS} {}

{REG} { std::string s;
        if(strlen(yytext)==3){
                s=yytext[2];
        }else{
                s=yytext[2]+yytext[3];
        }
        yylval.a=new Argument("reg",stoi(s,0,16),yytext);
        return REG;}

{PC} {yylval.a=new Argument("reg",15,yytext);
        return REG;}
{SP} {yylval.a=new Argument("reg",14,yytext);
        return REG;}

{TO} {return TO;}

{EOL} {return EOL;}

{COMMA} {return COMMA;}

{LPAR} {return LPAR;}

{RPAR} {return RPAR;}

{EQ} {return EQ;} /* = */

{IMM} {return IMM;} /*for immediate values $ */

{PER} {return PER;} /*for values in registers % */

{PLUS} {return PLUS;}

%%

int yywrap(){return 0;}

