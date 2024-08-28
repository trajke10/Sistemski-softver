#ifndef LINKER_HPP
#define LINKER_HPP

#include "File.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <map>
using namespace std;

class Linker
{

private:

  vector<Section *> tableOfSections;
  vector<Symbol *> tableOfSymbols;

  vector<File *> files;
  string typeOfLinking;

  vector<unsigned char> code;
  vector<ifstream> finputs;
  ofstream foutput;
  string outFileName;
  long locationCounter;
  vector<string> definedSymbols;
  vector<string> undefinedSymbols;

  map<string,string> sectionToAddress;

  bool flag = false;

public:
  static Linker &getLinker()
  {
    static Linker instance;
    if (!instance.flag)
    {
      instance.flag = true;
      instance.tableOfSymbols.push_back(new Symbol(0,0,0,"NOTYP",'l',0,"UND"));
    }
    return instance;
  }

  void initialize(vector<string>inputFiles,string typeOfL,string output,map<string, string> sectionToAdd);

  void startLinking();

  void parseFile(int i);

  Section *getSectionByName(string name);

  Symbol *getSymbol(int rb,vector<Symbol*>* symbols);

  int getRbFromGlobalSymbolTable(char bind,int ndx,string name);

  void addToTableOfSections(vector<Section*>* pomTSec);

  void createTableOfSymbols();

  bool checkForOverlapping(Section* sec);

  void addCodeToSection(vector<unsigned char> *code, Section *sec);

  bool existsInD(string name);

  void addToUndefinedSymbols(string name);

  void solveRellocations();

  void removeFromUndefinedSymbols(string name);

  void orderSectionsByBeginAddress();

  void finishLinking();

  bool findInTable(int ndx, string name);

  void createTextFile();

  void createHexObjectFile();

  void createRelocatableObjectFile();
};

#endif