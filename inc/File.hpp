#ifndef FAJL_HPP
#define FAJL_HPP

#include "Section.hpp"
#include "Symbol.hpp"
#include <vector>
#include <iostream>
using namespace std;

class File
{

private:

  vector<Section*> sections;
  vector<Symbol*> symbols;

public:

  File(vector<Section*> sec,vector<Symbol*> symb);

  vector<Section*> getSections();
  vector<Symbol*> getSymbols();

};

#endif