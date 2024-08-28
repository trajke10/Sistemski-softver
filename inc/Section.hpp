#ifndef SECTION_HPP
#define SECTION_HPP

#include <string>
#include <vector>
using namespace std;
#include "Literal.hpp"

struct Rellocation{
  long offset;
  string type;
  long symbol;
  long addend;

  Rellocation(long o,string t,long s,long a);
};


class Section{

public:

  string name;
  int size;
  int ndx;
  int numOfRellocations=0;
  vector<Rellocation*> relTable;
  vector<unsigned char> code;
  vector<Literal*> tableOfLiterals;
  unsigned long offsetInObjectFile;
  unsigned indexOfFile;

  Section(string n,int s,int nd){
    name=n;
    size=s;
    ndx=nd;
    offsetInObjectFile=0;
  }

  void addRellocation(Rellocation* r);

  void addLiteral(Literal* l);

  Literal* existsInLiteralTable(long num);

  void setOffsetInObjectFile(unsigned long o);

  unsigned long getOffsetInObjectFile();

  void setIndexFile(unsigned int i);

  unsigned int getIndexFile();

  int getNumOfRellocations();

  void setNumOfRellocations(int n);

};

#endif