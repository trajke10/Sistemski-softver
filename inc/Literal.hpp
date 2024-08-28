#ifndef LITERAL_HPP
#define LITERAL_HPP
#include "Symbol.hpp"

class Literal{

public:

  long num;
  int size;
  Position* positinsOfUse;

  Literal(long n,int s,Position* p):num(n),size(s),positinsOfUse(p){}

};

#endif
