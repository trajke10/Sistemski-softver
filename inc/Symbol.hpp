#ifndef SYMBOL_HPP
#define SYMBOL_HPP

#include <string>
using namespace std;

struct Position
{
  int loc;
  Position *next;
  string typeOfUse; //absolute or pc relative
  int secNdx;
  Position(int l,string tou,int s):loc(l),typeOfUse(tou),secNdx(s){};
};

class Symbol
{

public:

  int num;
  int value;
  int size;
  string type;
  char bind;
  int ndx; //0:extern -1:not defined >0:defined in section ndx
  string name;
  Position *positionsOfUse;

  Symbol(int n, int v, int s, string t, char b, int nd, string na);

};

#endif