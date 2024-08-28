#include "../inc/Linker.hpp"

void Linker::initialize(vector<string> inputFiles, string typeOfL, string output, map<string, string> sectionToAdd)
{
  typeOfLinking = typeOfL;
  outFileName = output;
  sectionToAddress = sectionToAdd;

  for (int i = 0; i < inputFiles.size(); i++)
  {
    finputs.push_back(ifstream(inputFiles[i], ios::in | ios::binary));
    if (!finputs[i].is_open())
    {
      cout << "Unable to open file " << inputFiles[i] << endl;
      exit(-1);
    }
  }
}

void Linker::startLinking()
{
  for (int i = 0; i < finputs.size(); i++)
  {
    parseFile(i);
  }
  // sections with code are created

  createTableOfSymbols();
  finishLinking();
}

void Linker::parseFile(int i)
{

  vector<Section *> pomTSec;
  vector<Symbol *> pomTSymb;

  int numOfSections;
  string nameOfSection;
  int sizeOfSection;
  int ndxSection;
  int numOfRellocations;

  finputs[i].read((char *)(&numOfSections), sizeof(numOfSections));

  for (int j = 0; j < numOfSections; j++)
  {
    int length;
    finputs[i].read((char *)(&length), sizeof(length));
    nameOfSection.resize(length);
    finputs[i].read((char *)(nameOfSection.c_str()), length);

    finputs[i].read((char *)(&sizeOfSection), sizeof(sizeOfSection));

    finputs[i].read((char *)(&ndxSection), sizeof(ndxSection));

    finputs[i].read((char *)(&numOfRellocations), sizeof(numOfRellocations));

    Section *nS = new Section(nameOfSection, sizeOfSection, ndxSection);
    nS->setIndexFile(i);
    nS->setOffsetInObjectFile(0);
    nS->setNumOfRellocations(numOfRellocations);
    pomTSec.push_back(nS);
  }

  int numOfSymbols;
  int num;
  int value;
  int size;
  string type;
  char bind;
  int ndx;
  string name;

  finputs[i].read((char *)(&numOfSymbols), sizeof(numOfSymbols));
  for (int j = 0; j < numOfSymbols; j++)
  {
    finputs[i].read((char *)(&num), sizeof(num));

    finputs[i].read((char *)(&value), sizeof(value));

    finputs[i].read((char *)(&size), sizeof(size));

    int length;
    finputs[i].read((char *)(&length), sizeof(length));
    type.resize(length);
    finputs[i].read((char *)(type.c_str()), length);

    finputs[i].read(&bind, 1);

    finputs[i].read((char *)(&ndx), sizeof(ndx));

    finputs[i].read((char *)(&length), sizeof(length));
    name.resize(length);
    finputs[i].read((char *)(name.c_str()), length);

    Symbol *s = new Symbol(num, value, size, type, bind, ndx, name);
    pomTSymb.push_back(s);
  }

  // relokacioni zapisi

  for (auto s : pomTSec)
  {

    for (int k = 0; k < s->getNumOfRellocations(); k++)
    {
      long offset;
      string type;
      long symbol;
      long addend;

      finputs[i].read((char *)(&offset), sizeof(offset));

      int length;
      finputs[i].read((char *)(&length), sizeof(length));
      type.resize(length);
      finputs[i].read((char *)(type.c_str()), length);

      finputs[i].read((char *)(&symbol), sizeof(symbol));

      finputs[i].read((char *)(&addend), sizeof(addend));

      Rellocation *r = new Rellocation(offset, type, symbol, addend);
      s->relTable.push_back(r);
    }
  }
  // kod
  for (auto s : pomTSec)
  {
    unsigned char buffer[s->size];
    finputs[i].read(reinterpret_cast<char *>(buffer), sizeof(buffer));
    for (int k = 0; k < s->size; k++)
    {
      s->code.push_back(buffer[k]);
    }
  }

  // sada je sve spremno u pomocnim strukturama
  files.push_back(new File(pomTSec, pomTSymb));

  addToTableOfSections(&pomTSec);
}

Section *Linker::getSectionByName(string name)
{
  for (auto s : tableOfSections)
  {
    if (s->name == name)
      return s;
  }
  return nullptr;
}

Symbol *Linker::getSymbol(int rb, vector<Symbol *> *symbols)
{
  for (auto s : *symbols)
  {
    if (s->num == rb)
      return s;
  }
  return nullptr;
}

int Linker::getRbFromGlobalSymbolTable(char bind, int ndx, string name)
{
  for (auto s : tableOfSymbols)
  {
    if (bind == 'l')
    {
      if (s->bind == bind && s->ndx == ndx && s->name == name)
        return s->num;
    }
    else
    {
      if (s->bind == bind && s->name == name)
        return s->num;
    }
  }
  return 0;
}

void Linker::addToTableOfSections(vector<Section *> *pomTSec)
{

  for (auto s : *pomTSec)
  {
    Section *sec = getSectionByName(s->name);
    if (!sec)
    {
      int nextIndexSec = tableOfSections.size() + 1;
      sec = new Section(s->name, s->size, nextIndexSec);
      sec->setOffsetInObjectFile(0);
      tableOfSections.push_back(sec);
      int nextIndexSymb = tableOfSymbols.size();
      tableOfSymbols.push_back(new Symbol(nextIndexSymb, 0, 0, "SCTN", 'l', nextIndexSec, s->name));
    }
    else
    {
      s->setOffsetInObjectFile(sec->size);
      sec->size += s->size;
    }
    addCodeToSection(&(s->code), sec);
  }
}

void Linker::createTableOfSymbols()
{
  if (typeOfLinking == "-hex")
  {

    long endOfLastSection = 0;
    for (int i = 0; i < tableOfSections.size(); i++)
    {

      string address = sectionToAddress[tableOfSections[i]->name];
      long offset;
      if (address == "")
      { // nije definisan pocetak place opcijom
        continue;
      }
      else
      {
        offset = stol(address, 0, 16);
      }

      if (offset + tableOfSections[i]->size > 4294967295)
      {
        cout << "LD ERROR: Section " << tableOfSections[i]->name << " can't be placed on specified position!\n";
        exit(-1);
      }

      if (offset + tableOfSections[i]->size > endOfLastSection)
      {
        endOfLastSection = offset + tableOfSections[i]->size;
      }

      tableOfSections[i]->setOffsetInObjectFile(offset);
    }

    for (int i = 0; i < tableOfSections.size(); i++)
    {

      string address = sectionToAddress[tableOfSections[i]->name];
      long offset;
      if (address != "")
      { // sekcija vec smestena zbog place opcije
        continue;
      }
      else
      {
        offset = endOfLastSection;
      }

      if (offset + tableOfSections[i]->size > 4294967295)
      {
        cout << "LD ERROR: Section " << tableOfSections[i]->name << " can't be placed on the end of file!\n";
        exit(-1);
      }

      tableOfSections[i]->setOffsetInObjectFile(offset);
      endOfLastSection = offset + tableOfSections[i]->size;
    }

    for (auto s : tableOfSections)
    {
      if (checkForOverlapping(s))
      {
        cout << "LD ERROR: Sections are overlapping\n";
        exit(-1);
      }
    }

    for (auto s : tableOfSymbols)
    {
      if (s->name == "UND")
        continue;
      s->value = getSectionByName(s->name)->getOffsetInObjectFile();
    }
  }

  vector<Section *> sections;
  vector<Symbol *> symbols;

  for (auto f : files)
  {

    sections = f->getSections();
    symbols = f->getSymbols();

    for (auto symb : symbols)
    {
      bool ext = false;
      if (symb->type == "SCTN") // sekcije su vec ubacene
        continue;

      bool exists = existsInD(symb->name);

      if (symb->bind == 'g')
      {
        if (symb->ndx != 0)
        { // simbol je definisan i izvezen
          if (exists)
          { // simbol ima dvostruku definiciju
            cout << "LD ERROR: Symbol " << symb->name << " has double definition\n";
            exit(-1);
          }
          else
          {
            definedSymbols.push_back(symb->name);
            removeFromUndefinedSymbols(symb->name);
            exists = true;
          }
        }
        else
        { // simbol se uvozi
          if (!exists)
            addToUndefinedSymbols(symb->name);
          ext = true;
        }
      }

      if (!ext)
      {
        int ndx = symb->ndx;
        string secName;
        long offsetInObjectFile = 0;
        for (auto sec : sections)
        {
          if (sec->ndx == ndx)
          {
            secName = sec->name;
            offsetInObjectFile = sec->offsetInObjectFile;
          }
        }
        Section *sec = getSectionByName(secName);
        // provera da li u datoj sekciji vec postoji definicija istoimenog simbola
        bool flag = findInTable(sec->ndx, symb->name);
        int nextIndex = tableOfSymbols.size();
        if (typeOfLinking == "-hex")
        { // ipak je potrebna virtuelna adresa
          offsetInObjectFile += sec->getOffsetInObjectFile();
        }
        tableOfSymbols.push_back(new Symbol(nextIndex, offsetInObjectFile + symb->value, 0, "NOTYP", symb->bind, sec->ndx, symb->name));
      }
    }
  }
}

bool Linker::checkForOverlapping(Section *sec)
{

  int beginOfSec = sec->getOffsetInObjectFile();
  int endOfSec = sec->getOffsetInObjectFile() + sec->size - 1;

  for (auto s : tableOfSections)
  {
    if (sec->name == s->name)
      continue;
    if (beginOfSec >= s->getOffsetInObjectFile() && beginOfSec < s->getOffsetInObjectFile() + s->size || endOfSec >= s->getOffsetInObjectFile() && endOfSec < s->getOffsetInObjectFile() + s->size)
    {
      return true;
    }
  }
  return false;
}

void Linker::addCodeToSection(vector<unsigned char> *code, Section *sec)
{
  for (auto co : *code)
  {
    sec->code.push_back(co);
  }
}

bool Linker::existsInD(string name)
{
  for (auto s : definedSymbols)
  {
    if (s == name)
      return true;
  }
  return false;
}

void Linker::addToUndefinedSymbols(string name)
{
  for (auto u : undefinedSymbols)
  {
    if (u == name)
      return;
  }
  undefinedSymbols.push_back(name);
}

void Linker::solveRellocations()
{

  // najpre prevodjenje starih rel zapisa u nove

  vector<Section *> sections;
  vector<Symbol *> symbols;
  for (auto f : files)
  {
    sections = f->getSections();
    symbols = f->getSymbols();

    for (auto s : sections)
    {
      Section *secForRel = getSectionByName(s->name);

      for (auto r : s->relTable)
      {
        Symbol *symb = getSymbol(r->symbol, &symbols);

        if (symb->type == "SCTN")
        {
          Section *sec = getSectionByName(symb->name);
          int rb = getRbFromGlobalSymbolTable(symb->bind, sec->ndx, symb->name);
          secForRel->addRellocation(new Rellocation(s->offsetInObjectFile + r->offset, r->type, rb, s->offsetInObjectFile + r->addend));
        }
        else
        {
          int rb = getRbFromGlobalSymbolTable(symb->bind, symb->ndx, symb->name);
          secForRel->addRellocation(new Rellocation(s->offsetInObjectFile + r->offset, r->type, rb, r->addend));
        }
      }
    }
  }
  // sada resavanje rel zapisa
  if (typeOfLinking == "-hex")
  {
    for (auto s : tableOfSections)
    {
      for (auto r : s->relTable)
      {
        Symbol *symb = getSymbol(r->symbol, &tableOfSymbols);
        unsigned int relV = r->addend + symb->value;
        int relL = r->offset;
        while (relV)
        {
          s->code[relL] = relV;
          relV >>= 8;
          relL++;
        }
      }
    }
  }
}

void Linker::removeFromUndefinedSymbols(string name)
{
  vector<string>::iterator it;
  for (it = undefinedSymbols.begin(); it < undefinedSymbols.end(); it++)
  {
    if (*it == name)
    {
      undefinedSymbols.erase(it);
    }
  }
}

void Linker::orderSectionsByBeginAddress()
{

  for (int i = 0; i < tableOfSections.size() - 1; i++)
  {
    for (int j = i + 1; j < tableOfSections.size(); j++)
    {
      Section *pom;
      if (tableOfSections[i]->getOffsetInObjectFile() > tableOfSections[j]->getOffsetInObjectFile())
      {
        pom = tableOfSections[i];
        tableOfSections[i] = tableOfSections[j];
        tableOfSections[j] = pom;
      }
    }
  }
}

void Linker::finishLinking()
{

  if (undefinedSymbols.size() && typeOfLinking == "-hex")
  {
    for (auto s : undefinedSymbols)
    {
      cout << "LD ERROR: Symbol " << s << " is not defined in any file\n";
    }
    exit(-1);
  }

  if (undefinedSymbols.size() && typeOfLinking == "-relocatable")
  {
    for (auto s : undefinedSymbols)
    {
      int nextIndex = tableOfSymbols.size();
      tableOfSymbols.push_back(new Symbol(nextIndex, 0, 0, "NOTYP", 'g', 0, s));
    }
  }

  solveRellocations(); // u okviru same fje se proverava da li se rel zapisi razresavaju potpuno ili se samo menjaju

  orderSectionsByBeginAddress();

  if (typeOfLinking == "-hex")
  {
    createHexObjectFile();
  }
  else
  {
    createRelocatableObjectFile();
  }
  createTextFile();

  cout << "Finished linking\n";
}

bool Linker::findInTable(int ndx, string name)
{
  for (auto s : tableOfSymbols)
  {
    if (s->ndx == ndx && s->name == name)
      return true;
  }
  return false;
}

void Linker::createTextFile()
{
  ofstream textOutput("linker.txt");

  textOutput << "Table of symbols: \n";

  textOutput << "Num   Value   Size   Type   Bind   Ndx   Name\n";

  for (int i = 0; i < tableOfSymbols.size(); i++)
  {
    textOutput << tableOfSymbols.at(i)->num << " " << hex << tableOfSymbols.at(i)->value << " " << dec << tableOfSymbols.at(i)->size << " " << tableOfSymbols.at(i)->type << " " << tableOfSymbols.at(i)->bind << " " << tableOfSymbols.at(i)->ndx << " " << tableOfSymbols.at(i)->name << "\n";
  }

  textOutput << "\nTable of sections: \n";

  textOutput << "Name   Ndx   Address   Size\n";

  for (auto s : tableOfSections)
  {
    textOutput << s->name << " " << s->ndx << " " << hex << s->getOffsetInObjectFile() << " " << dec << s->size << endl;
  }

  if (typeOfLinking == "-relocatable")
  {
    for (auto s : tableOfSections)
    {

      textOutput << "\nRella table " << s->name << ":" << endl;
      if (s->relTable.size() == 0)
      {
        textOutput << "No rellocations\n";
      }
      else
      {
        textOutput << "Offset   Type   Symbol   Addend\n";
      }

      for (auto r : s->relTable)
      {

        textOutput << hex << r->offset << " ";

        textOutput << r->type << " ";

        textOutput << r->symbol << " ";

        textOutput << r->addend << endl;
      }
    }
  }

  for (auto s : tableOfSections)
  {
    int j = 0;
    textOutput << "\nSection " << s->name << "\n";

    for (int i = 0; i < s->code.size(); i++)
    {
      if (j % 4 == 0 && j < s->code.size())
      {
        textOutput << "\n";
        textOutput << setw(2) << setfill('0') << j << ": ";
      }
      textOutput << setw(2) << setfill('0') << hex << (unsigned int)s->code[i] << " ";
      j++;
    }
    textOutput << "\n";
  }
  textOutput << "\n";

  textOutput << "Machine code of program\n\n";

  for (auto s : tableOfSections)
  {
    unsigned char buffer[s->size];
    for (int i = 0; i < s->size; i++)
    {
      buffer[i] = s->code[i];
    }
    int k = 0;
    unsigned char pom[8];

    for (long i = s->getOffsetInObjectFile(); i < s->getOffsetInObjectFile() + s->size; i += 8)
    {
      textOutput << hex << setw(8) << setfill('0') << i << ": ";

      for (int j = 0; j < 8; j++)
      {
        textOutput << setw(2) << setfill('0') << hex << (unsigned int)buffer[k++] << " ";
      }
      textOutput << endl;
    }
  }

  textOutput.close();
}

void Linker::createHexObjectFile()
{
  foutput = ofstream(outFileName, ios::binary | ios::out);

  if (!foutput.is_open())
  {
    cout << "Unable to open file " << outFileName << " for writing\n";
    exit(-1);
  }

  for (auto s : tableOfSections)
  {

    unsigned char buffer[s->size];
    for (int i = 0; i < s->size; i++)
    {
      buffer[i] = s->code[i];
    }
    int k = 0;
    unsigned char pom[8];

    for (long i = s->getOffsetInObjectFile(); i < s->getOffsetInObjectFile() + s->size; i += 8)
    {
      foutput << hex << setw(8) << setfill('0') << i << ": ";
      int numOfWritedElements = 0;
      for (int j = 0; j < 8; j++)
      {
        numOfWritedElements++;
        pom[j] = buffer[k++];
      }
      foutput.write(reinterpret_cast<char *>(pom), numOfWritedElements);
      foutput << endl;
    }
  }

  foutput.close();
}

void Linker::createRelocatableObjectFile()
{
  foutput = ofstream(outFileName, ios::out | ios::binary);
  if (foutput.is_open())
  {
    int asa = tableOfSections.size();
    foutput.write((char *)(&asa), sizeof(asa));

    for (auto s : tableOfSections)
    {
      int length = s->name.size();
      foutput.write((char *)(&length), sizeof(length));

      foutput.write((s->name).c_str(), length);

      foutput.write((char *)(&s->size), sizeof(s->size));

      foutput.write((char *)(&s->ndx), sizeof(s->ndx));

      foutput.write((char *)(&s->numOfRellocations), sizeof(s->numOfRellocations));
    }

    int ss = tableOfSymbols.size() - 1;
    foutput.write((char *)(&ss), sizeof(ss));

    // tabela simbola
    for (auto s : tableOfSymbols)
    {
      if (s->name == "UND")
        continue;
      // foutput << s->num << " " << s->value << " " << s->size << " " << s->type << " " << s->bind << " " << s->ndx << " " << s->name << endl;
      foutput.write((char *)(&s->num), sizeof(s->num));

      foutput.write((char *)(&s->value), sizeof(s->value));

      foutput.write((char *)(&s->size), sizeof(s->size));

      int length = s->type.size();
      foutput.write((char *)(&length), sizeof(length));
      foutput.write((s->type).c_str(), length);

      foutput.write((char *)(&s->bind), sizeof(s->bind));

      foutput.write((char *)(&s->ndx), sizeof(s->ndx));

      length = s->name.size();
      foutput.write((char *)(&length), sizeof(length));
      foutput.write((s->name).c_str(), length);
    }

    // tabela relokacija
    for (auto s : tableOfSections)
    {
      for (auto r : s->relTable)
      {

        foutput.write((char *)(&r->offset), sizeof(r->offset));

        int length = r->type.size();
        foutput.write((char *)(&length), sizeof(length));
        foutput.write((r->type).c_str(), length);

        foutput.write((char *)(&r->symbol), sizeof(r->symbol));

        foutput.write((char *)(&r->addend), sizeof(r->addend));
      }
    }

    // kod sekcija
    for (auto s : tableOfSections)
    {
      unsigned char buffer[s->size];
      int k = 0;
      for (auto c : s->code)
      {
        buffer[k++] = c;
      }
      foutput.write(reinterpret_cast<char *>(buffer), sizeof(buffer));
    }

    foutput.close();
  }
  else
  {
    cout << "Could not open file\n";
    exit(-1);
  }
}
