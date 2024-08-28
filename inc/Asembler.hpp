#ifndef ASEMBLER_HPP
#define ASEMBLER_HPP

#include <string.h>
using namespace std;
#include <vector>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <map>
#include "Symbol.hpp"
#include "Section.hpp"
#include "Literal.hpp"
#include <sstream>


extern int yyparse();
extern FILE* yyin;

struct Argument
{
  string type;
  long num_value;
  string symb_value;
  Argument* next;

  Argument(string t,long v, string s);
};

enum INSTRUCTION{JMP,BEQ,BNE,BGT,ADD,SUB,MUL,DIV,NOT,AND,OR,XOR,SHL,SHR};

class Asembler{

  private:

    bool flag=false;
    int curNumOfSymbols;
    int locationCounter;
    vector<Symbol*> tableOfSymbols;
    vector<Section*> tableOfSections;
    Section* curSection;
    char* inputFile;
    char* outputFile;
    FILE* finput;
    ofstream foutput;

    ofstream textOutput;

  public:

  static Asembler& getAsembler(){
    static Asembler instance;
    if(!instance.flag){
      instance.curNumOfSymbols=0;
      instance.flag=true;
      instance.locationCounter=0;
      instance.curSection=nullptr;
      instance.tableOfSymbols.push_back(new Symbol(0,0,0,"NOTYP",'l',0,"UND"));
      instance.curNumOfSymbols++;
    }
    return instance;
  }

  void initialize(char* outputFile,char* inputFile);

  void addSymbol(string s);

  void processDirective(string name,Argument* args);

  void createInstructionHalt();

  void createInstructionInt();

  void createInstructionCall(Argument* arg);

  void createInstructionJump(string name,Argument* arg1,Argument* arg2,Argument* arg3);

  void helperForBranch(unsigned char prefix, unsigned char pn, unsigned char ps, Argument *arg1, Argument *arg2, Argument *arg3);
  
  void createArithmeticInstruction(string name,Argument* arg1,Argument* arg2);

  void createLogicalInstruction(string name,Argument* arg1,Argument* arg2);

  void createShiftInstruction(string name,Argument* arg1,Argument* arg2);

  void createXchgInstruction(Argument* arg1,Argument* arg2);

  void createLdInstruction(Argument* arg1,Argument* arg2,Argument* arg3);

  void createStInstruction(Argument* arg1,Argument* arg2,Argument* arg3);

  void createCsrInstruction(string rw,Argument* arg1,Argument* arg2);

  void createPushInstruction(Argument* arg1);

  void createPopInstruction(Argument* arg1,unsigned char pom=4);

  void createIretInstruction();

  void finishAssembling();

  void createTextFile();

  Symbol* existsInSymbolTable(string s);

  void createLiteralPools();

  void createRellocationTableForSections();

  Section* getSectionWithNdx(int ndx);

  void checkSymbols();

  void createObjectFile();

};

#endif