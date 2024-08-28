#include "../inc/Symbol.hpp"

Symbol::Symbol(int n,int v,int s,string t,char b,int nd, string na){
    num=n;
    value=v;
    size=s;
    type=t;
    bind=b;
    ndx=nd;
    name=na;
    positionsOfUse=nullptr;
}