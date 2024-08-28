#include "../inc/Section.hpp"

Rellocation:: Rellocation(long o,string t,long s,long a){
    offset=o;
    type=t;
    symbol=s;
    addend=a;
}

void Section::addRellocation(Rellocation* r){
    relTable.push_back(r);
    numOfRellocations++;
}

void Section::addLiteral(Literal *l){
    tableOfLiterals.push_back(l);
}

Literal *Section::existsInLiteralTable(long num){
    for(int i=0;i<tableOfLiterals.size();i++){
        if(tableOfLiterals.at(i)->num==num && tableOfLiterals.at(i)->size==sizeof(num))
        return tableOfLiterals.at(i);
    }    
    return nullptr;
}

void Section::setOffsetInObjectFile(unsigned long o)
{
    offsetInObjectFile=o;
}

unsigned long Section::getOffsetInObjectFile()
{
  return offsetInObjectFile;
}

void Section::setIndexFile(unsigned int i)
{
    indexOfFile=i;
}

unsigned int Section::getIndexFile()
{
  return indexOfFile;
}

int Section::getNumOfRellocations()
{
  return numOfRellocations;
}

void Section::setNumOfRellocations(int n)
{
    numOfRellocations=n;
}
