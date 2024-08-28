#include <stdio.h>
#include "../misc/parser.hpp"
#include "../misc/lexer.hpp"
#include "../inc/Asembler.hpp"

int main(int argc,char* argv[]){

  if(argc>4){
    cout<<"To many arguments for asembler\n";
    exit(-1);
  }else if(argc<4){
    cout<<"To few arguments for asembler\n";
    exit(-1);  
  }

  if(strcmp(argv[1],"-o")!=0){
    cout<<"Name of assembling output not defined!\n";
    exit(-1);
  }
  
  Asembler::getAsembler().initialize(argv[2],argv[3]);

  return 0;

}