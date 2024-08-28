#include "../inc/File.hpp"

File::File(vector<Section *> sec, vector<Symbol *> symb)
{
  sections=sec;
  symbols=symb;
}

vector<Section *> File::getSections()
{
  return sections;
}

vector<Symbol *> File::getSymbols()
{
  return symbols;
}
