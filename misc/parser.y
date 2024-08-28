%{
  /* definitions */
  #include <stdio.h>
  #include "../inc/Asembler.hpp"
  #include <string.h>
  #include "lexer.hpp"

  void yyerror(const char* s);

%}


%union{
  struct Argument* a;
}

%output "./misc/parser.cpp"
%defines "./misc/parser.hpp"

%token EOL
%token COMMA
%token LPAR
%token RPAR 
%token EQ
%token IMM
%token PER
%token TO
%token PLUS
%token HALT_T
%token INT_T
%token IRET_T
%token CALL_T
%token RET_T
%token JMP_T
%token BEQ_T
%token BNE_T
%token BGT_T
%token PUSH_T
%token POP_T
%token XCHG_T
%token ADD_T
%token SUB_T
%token MUL_T
%token DIV_T
%token NOT_T
%token AND_T
%token OR_T
%token XOR_T
%token SHL_T
%token SHR_T
%token LD_T
%token ST_T
%token CSRRD_T
%token CSRWR_T

%token<a> DIR
%token<a> IDENT
%token<a> IMMOPR
%token<a> REG
%token<a> CSR

%type row
%type<a> jmpoperand
%type<a> dataoperand
%type<a> argsOfDirective

%% /* pravila */

input:
| line input;

line:
row EOL
|labela EOL
|labela row EOL
|EOL;

row:
DIR argsOfDirective { //direktiva
  Argument* param;
  Argument* first=nullptr;
  Argument* prev=nullptr;
  while($2!=nullptr){
    param=new Argument($2->type,$2->num_value,$2->symb_value);
    if(prev!=nullptr){
      prev->next=param;
    }else{
      first=param;
    }
    prev=param;
    Argument* old=$2;
    $2=$2->next;
    free(old);
  }
  string st=$1->symb_value;
  free($1);
  Asembler::getAsembler().processDirective(st,first);
}
| HALT_T { Asembler::getAsembler().createInstructionHalt();}
| INT_T { Asembler::getAsembler().createInstructionInt();}
| IRET_T {Asembler::getAsembler().createIretInstruction();}
| CALL_T jmpoperand {
  Asembler::getAsembler().createInstructionCall($2);
}
| RET_T {
  Asembler::getAsembler().createPopInstruction(new Argument("reg",15,"x"));
}
| JMP_T  jmpoperand{
  Asembler::getAsembler().createInstructionJump("JMP",$2,nullptr,nullptr);
  }
| BEQ_T REG COMMA REG COMMA jmpoperand {
  Asembler::getAsembler().createInstructionJump("BEQ",$2,$4,$6);
}
| BNE_T REG COMMA REG COMMA jmpoperand {
  Asembler::getAsembler().createInstructionJump("BNE",$2,$4,$6);
}
| BGT_T REG COMMA REG COMMA jmpoperand {
  Asembler::getAsembler().createInstructionJump("BGT",$2,$4,$6);
} 
| PUSH_T REG {Asembler::getAsembler().createPushInstruction($2);}
| POP_T REG {Asembler::getAsembler().createPopInstruction($2);}
| XCHG_T REG COMMA REG {Asembler::getAsembler().createXchgInstruction($2,$4);}
| ADD_T REG COMMA REG {Asembler::getAsembler().createArithmeticInstruction("ADD",$2,$4);}
| SUB_T REG COMMA REG {Asembler::getAsembler().createArithmeticInstruction("SUB",$2,$4);}
| MUL_T REG COMMA REG{Asembler::getAsembler().createArithmeticInstruction("MUL",$2,$4);}
| DIV_T REG COMMA REG {Asembler::getAsembler().createArithmeticInstruction("DIV",$2,$4);}
| NOT_T REG {Asembler::getAsembler().createLogicalInstruction("NOT",$2,nullptr);}
| AND_T REG COMMA REG {Asembler::getAsembler().createLogicalInstruction("AND",$2,$4);}
| OR_T REG COMMA REG {Asembler::getAsembler().createLogicalInstruction("OR",$2,$4);}
| XOR_T REG COMMA REG {Asembler::getAsembler().createLogicalInstruction("XOR",$2,$4);}
| SHL_T REG COMMA REG {Asembler::getAsembler().createShiftInstruction("SHL",$2,$4);}
| SHR_T REG COMMA REG {Asembler::getAsembler().createShiftInstruction("SHR",$2,$4);}
| LD_T dataoperand COMMA REG {
  Asembler::getAsembler().createLdInstruction($2,$2->next,$4);
}
| ST_T REG COMMA dataoperand{
  Asembler::getAsembler().createStInstruction($2,$4,$4->next);
}
| CSRRD_T CSR COMMA REG {Asembler::getAsembler().createCsrInstruction("read",$2,$4);}
| CSRWR_T REG COMMA CSR {Asembler::getAsembler().createCsrInstruction("write",$2,$4);}

labela:
IDENT TO{ //labela
  free($1);
  Asembler::getAsembler().addSymbol($1->symb_value);}

jmpoperand:
IMMOPR {$$=$1;}
| IDENT {$$=$1;}

dataoperand:
IMM IMMOPR {$$=$2;$$->type="$num";}
| IMM IDENT {$$=$2;$$->type="$symb";}
| IMMOPR {$$=$1;}
| IDENT {$$=$1;}
| REG {$$=$1;}
| LPAR REG RPAR {$$=$2;$$->type="[reg]";}
| LPAR REG PLUS IMMOPR RPAR {
  $$=$2;
  $$->next=$4;
  }
| LPAR REG PLUS IDENT RPAR {
  $$=$2;
  $$->next=$4;
  }

argsOfDirective:
{$$=nullptr;}
|jmpoperand {
  $$=$1;
}
| jmpoperand COMMA argsOfDirective {
  $$=new Argument($1->type,$1->num_value,$1->symb_value);
  $$->next=$3;
  free($1);
}

%%

void yyerror(const char* s){
  printf("ERROR: %s\n",s);
}
