#include "../inc/Asembler.hpp"

Argument::Argument(string t, long v, string s)
{
  this->type = t;
  this->num_value = v;
  this->symb_value = s;
  this->next = nullptr;
}

map<string, INSTRUCTION> instructionToString = {{"JMP", JMP},
                                                {"BEQ", BEQ},
                                                {"BNE", BNE},
                                                {"BGT", BGT},
                                                {"ADD", ADD},
                                                {"SUB", SUB},
                                                {"MUL", MUL},
                                                {"DIV", DIV},
                                                {"NOT", NOT},
                                                {"AND", AND},
                                                {"OR", OR},
                                                {"XOR", XOR},
                                                {"SHL", SHL},
                                                {"SHR", SHR}};

INSTRUCTION stringToEnum(string s)
{
  return instructionToString[s];
}

void Asembler::initialize(char *outputFile, char *inputFile)
{
  this->inputFile = inputFile;
  this->outputFile = outputFile;
  finput = fopen(inputFile, "r");
  if (!finput)
  {
    cout << "ERROR: Could not open specified file\n";
    exit(-1);
  }
  else
  {
    yyin = finput;
    if (yyparse() != 0)
    {
      cout << "ERROR: Parsing failed\n";
      fclose(finput);
      exit(-1);
    }
    fclose(finput);
    exit(0);
  }
}

void Asembler::addSymbol(string s)
{

  bool found = false;

  Symbol *symb = existsInSymbolTable(s);
  if (symb != nullptr)
  {
    if (symb->ndx > 0)
    {
      cout << "AS ERROR (" << inputFile << "): Double definition of symbol " << s << "\n";
      exit(-1);
    }
    else if (symb->ndx == 0)
    {
      cout << "AS ERROR (" << inputFile << "): Definition of extern symbol " << s << "\n";
      exit(-1);
    }
    else
    { // nor extern nor defined
      symb->ndx = curSection->ndx;
      if (symb->bind != 'g') // moze da se desi da je simbol ubacen u tabelu zbog obracanja unapred ili zbog direktive .global
        symb->bind = 'l';
      symb->value = locationCounter;
      found = true;
    }
  }

  if (!found)
  {
    Symbol *st = new Symbol(curNumOfSymbols++, locationCounter, 0, "NOTYP", 'l', curSection->ndx, s);
    tableOfSymbols.push_back(st);
  }
}

void Asembler::processDirective(string name, Argument *args)
{

  Argument *first = args;
  if (!args && name != ".end")
  {
    cout << "AS ERROR (" << inputFile << "): No arguments given for directive " << name << "\n";
    exit(-1);
  }

  if (name == ".global")
  {

    bool found;

    while (first != nullptr)
    {
      found = false;
      if (first->type == "num")
      {
        cout << "AS ERROR (" << inputFile << "): Invalid argument given for .global directive!" << "\n";
        exit(-1);
      }

      Symbol *symb = existsInSymbolTable(first->symb_value);
      if (symb != nullptr)
      {
        if (symb->ndx == 0)
        {
          cout << "AS ERROR (" << inputFile << "): Symbol " << symb->name << " first marked as extern then as global" << "\n";
          exit(-1);
        }
        else
        {
          symb->bind = 'g';
          found = true;
        }
      }
      if (!found)
      {
        Symbol *st = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", 'g', -1, first->symb_value);
        tableOfSymbols.push_back(st);
      }
      Argument *old = first;
      first = first->next;
      free(old);
    }
  }
  else if (name == ".extern")
  {

    first = args;

    while (first)
    {

      Symbol *symb = existsInSymbolTable(first->symb_value);
      if (symb != nullptr)
      {
        if (symb->ndx != -1)
        {
          cout << "AS ERROR (" << inputFile << "): Symbol " << symb->name << " already defined" << "\n";
          exit(-1);
        }
        else
        {
          cout << "AS ERROR (" << inputFile << "): Symbol " << symb->name << " first marked as global then as extern" << "\n";
          exit(-1);
        }
      }

      Symbol *st = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", 'g', 0, first->symb_value);
      tableOfSymbols.push_back(st);

      Argument *old = first;
      first = first->next;
      free(old);
    }
  }
  else if (name == ".section")
  {

    first = args;

    if (args->next)
    {
      cout << "AS ERROR (" << inputFile << "): To many arguments for directive .section" << "\n";
      exit(-1);
    }

    Symbol *symb = existsInSymbolTable(first->symb_value);
    if (symb != nullptr)
    {
      cout << "AS ERROR (" << inputFile << "):Section already exists!" << "\n";
      exit(-1);
    }

    if (curSection != nullptr)
    {
      for (int i = 0; i < tableOfSymbols.size(); i++)
      {
        if (tableOfSymbols.at(i)->name == curSection->name)
        {
          curSection->size = locationCounter;
          locationCounter = 0;
        }
      }
    }

    Symbol *st = new Symbol(curNumOfSymbols++, 0, 0, "SCTN", 'l', curNumOfSymbols - 1, first->symb_value);
    curSection = new Section(first->symb_value, 0, curNumOfSymbols - 1);
    tableOfSymbols.push_back(st);
    tableOfSections.push_back(curSection);
    free(first);
  }
  else if (name == ".word")
  {

    while (first)
    {

      if (first->type == "num")
      { // literal

        unsigned char c1 = first->num_value & 0xFF;
        unsigned char c2 = (first->num_value >> 8) & 0xFF;
        unsigned char c3 = (first->num_value >> 8) & 0xFF;
        unsigned char c4 = (first->num_value >> 8) & 0xFF;
        curSection->code.push_back(c1);
        curSection->code.push_back(c2);
        curSection->code.push_back(c3);
        curSection->code.push_back(c4);
      }
      else
      { // simbol

        Symbol *s = existsInSymbolTable(first->symb_value);
        if (!s)
        {
          s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, first->symb_value);
          s->positionsOfUse = new Position(locationCounter, "32", curSection->ndx);
        }
        else
        {
          Position *first = s->positionsOfUse;
          if (!first)
          {
            s->positionsOfUse = new Position(locationCounter, "32", curSection->ndx);
          }
          else
          {
            while (first->next)
            {
              first = first->next;
            }
            first->next = new Position(locationCounter, "32", curSection->ndx);
          }
        }
        curSection->code.push_back(0);
        curSection->code.push_back(0);
        curSection->code.push_back(0);
        curSection->code.push_back(0);
      }
      locationCounter += 4;
      Argument *old = first;
      first = first->next;
      free(old);
    }
  }
  else if (name == ".skip")
  {

    first = args;

    if (first->next)
    {
      cout << "AS ERROR (" << inputFile << "): Directive .skip can have only one argument" << "\n";
      exit(-1);
    }

    if (first->type == "symb")
    {
      cout << "AS ERROR (" << inputFile << "): Argument for directive .skip can only be literal" << "\n";
      exit(-1);
    }

    for (int i = 0; i < first->num_value; i++)
    {
      curSection->code.push_back(0);
    }

    locationCounter += first->num_value;

    free(first);
  }
  else if (name == ".ascii")
  {

    if (args->next)
    {
      cout << "AS ERROR (" << inputFile << "): Directive .ascii can have only one argument\n";
      exit(-1);
    }
    if (args->type != "symb")
    {
      cout << "AS ERROR (" << inputFile << "): Argument for directive .ascii can only be string";
      exit(-1);
    }

    string s = args->symb_value;
    for (auto i : s)
    {
      curSection->code.push_back((int)i);
      locationCounter++;
    }
  }
  else if (name == ".end")
  {
    if (curSection != nullptr)
    {
      for (int i = 0; i < tableOfSymbols.size(); i++)
      {
        if (tableOfSymbols.at(i)->name == curSection->name)
        {
          curSection->size = locationCounter;
          locationCounter = 0;
        }
      }
    }
    finishAssembling();
  }
  else
  {

    cout << "AS ERROR (" << inputFile << "): Unrecognized directive" << "\n";
    exit(-1);
  }
}

void Asembler::createInstructionHalt()
{
  for (int i = 0; i < 4; i++)
    curSection->code.push_back(0);
  locationCounter += 4;
}

void Asembler::createInstructionInt()
{
  curSection->code.push_back(16);
  for (int i = 0; i < 3; i++)
    curSection->code.push_back(0);
  locationCounter += 4;
}

void Asembler::createInstructionCall(Argument *arg)
{

  if (arg->type == "symb")
  {

    curSection->code.push_back(33);
    curSection->code.push_back(15); // pc
    locationCounter += 2;
    Symbol *s = existsInSymbolTable(arg->symb_value);
    if (!s)
    {
      s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg->symb_value);
      s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx); // treba voditi racuna o polubajtu!!!
      tableOfSymbols.push_back(s);
      curSection->code.push_back(0);
      curSection->code.push_back(0);
      locationCounter += 2;
    }
    else if (s->ndx != curSection->ndx)
    { // sta ako je ovo pozvano pre prve sekcije???
      Position *firstP = s->positionsOfUse;
      if (!firstP)
      {
        s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      }
      else
      {
        while (firstP->next)
        {
          firstP = firstP->next;
        }
        firstP->next = new Position(locationCounter, "PC12", curSection->ndx);
      }
      curSection->code.push_back(0);
      curSection->code.push_back(0);
      locationCounter += 2;
    }
    else
    { // simbol je definisan i u istoj je sekciji
      curSection->code.pop_back();
      curSection->code.pop_back();
      curSection->code.push_back(32);
      curSection->code.push_back(15);
      long offset = s->value - locationCounter - 2;
      curSection->code.push_back(offset & 0xF);
      curSection->code.push_back((offset >> 4) & 0xFF);
      locationCounter += 2;
    }
  }
  else
  { // literal
    string t;
    if (arg->num_value <= 2047 && arg->num_value >= -2048)
    {
      curSection->code.push_back(32);
      curSection->code.push_back(15);
      t = "12";
    }
    else
    {
      curSection->code.push_back(33);
      curSection->code.push_back(15);
      t = "PC12";
    }
    locationCounter += 2; // treba voditi racuna o polubajtu!!!
    Literal *l = curSection->existsInLiteralTable(arg->num_value);
    if (!l)
      curSection->addLiteral(new Literal(arg->num_value, sizeof(arg->num_value), new Position(locationCounter, t, curSection->ndx)));
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, t, curSection->ndx);
    }
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 2;
  }
}

void Asembler::createInstructionJump(string name, Argument *arg1, Argument *arg2, Argument *arg3)
{

  unsigned char prefix = 3;
  prefix <<= 4;

  switch (stringToEnum(name))
  {
  case JMP:
    if (arg1->type == "symb")
    {
      curSection->code.push_back(8 | prefix);
      curSection->code.push_back(15 << 4);
      locationCounter += 2;
      Symbol *s = existsInSymbolTable(arg1->symb_value);
      if (!s)
      {
        s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg1->symb_value);
        s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx); // treba voditi racuna o polubajtu!!!
        tableOfSymbols.push_back(s);
        curSection->code.push_back(0);
        curSection->code.push_back(0);
        locationCounter += 2;
      }
      else if (s->ndx != curSection->ndx)
      { // sta ako je ovo pozvano pre prve sekcije???
        Position *firstP = s->positionsOfUse;
        if (!firstP)
        {
          s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
        }
        else
        {
          while (firstP->next)
          {
            firstP = firstP->next;
          }
          firstP->next = new Position(locationCounter, "PC12", curSection->ndx);
        }
        curSection->code.push_back(0);
        curSection->code.push_back(0);
        locationCounter += 2;
      }
      else
      {                              // simbol je definisan i u istoj je sekciji
        curSection->code.pop_back(); // nema potrebe za mem32[]
        curSection->code.pop_back();
        curSection->code.push_back(prefix);
        curSection->code.push_back(15 << 4);
        long offset = s->value - locationCounter - 2;
        curSection->code.push_back(offset & 0xF);
        curSection->code.push_back((offset >> 4) & 0xFF);
        locationCounter += 2;
      }
    }
    else
    { // literal
      string t;
      if (arg1->num_value <= 2047 && arg1->num_value >= -2048) // moze da stane u instrukciju
      {
        curSection->code.push_back(prefix);
        curSection->code.push_back(15 << 4);
        t = "12";
      }
      else
      {
        curSection->code.push_back(prefix | 8);
        curSection->code.push_back(15 << 4);
        t = "PC12";
      }
      locationCounter += 2; // treba voditi racuna o polubajtu!!!
      Literal *l = curSection->existsInLiteralTable(arg1->num_value);
      if (!l)
        curSection->addLiteral(new Literal(arg1->num_value, sizeof(arg1->num_value), new Position(locationCounter, t, curSection->ndx)));
      else
      {
        Position *first = l->positinsOfUse;
        while (first->next)
        {
          first = first->next;
        }
        first->next = new Position(locationCounter, t, curSection->ndx);
      }
      curSection->code.push_back(0);
      curSection->code.push_back(0);
      locationCounter += 2;
    }

    break;

  case BEQ:

    helperForBranch(prefix, 1, 9, arg1, arg2, arg3);

    break;

  case BNE:

    helperForBranch(prefix, 2, 10, arg1, arg2, arg3);

    break;

  case BGT:

    helperForBranch(prefix, 3, 11, arg1, arg2, arg3);

    break;

  default:
    cout << "Unrecognized instruction of jump";
    free(arg1);
    free(arg2);
    free(arg3);
    exit(-1);
  }

  free(arg1);
  free(arg2);
  free(arg3);
}

void Asembler::helperForBranch(unsigned char prefix, unsigned char pn, unsigned char ps, Argument *arg1, Argument *arg2, Argument *arg3)
{

  if (arg3->type == "num")
  {
    string t = "12";
    if (arg3->num_value > 2047 || arg3->num_value < -2048)
    {
      pn += 8;
      t = "PC12";
    }
    curSection->code.push_back(prefix | pn);
    curSection->code.push_back((arg1->num_value & 0xF) | (15 << 4));
    curSection->code.push_back((arg2->num_value << 4) & 0xFF);
    locationCounter += 2;
    Literal *l = curSection->existsInLiteralTable(arg3->num_value);
    if (!l)
      curSection->addLiteral(new Literal(arg3->num_value, sizeof(arg3->num_value), new Position(locationCounter, t, curSection->ndx)));
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, t, curSection->ndx);
    }
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else
  { // simbol
    curSection->code.push_back(prefix | ps);
    curSection->code.push_back((arg1->num_value & 0xF) | (15 << 4));
    curSection->code.push_back((arg2->num_value << 4) & 0xFF);
    locationCounter += 2;
    Symbol *s = existsInSymbolTable(arg3->symb_value);
    if (!s)
    {
      s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg3->symb_value);
      s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx); // treba voditi racuna o polubajtu!!!
      tableOfSymbols.push_back(s);
      curSection->code.push_back(0);
      locationCounter += 2;
    }
    else if (s->ndx != curSection->ndx || s->ndx == -1)
    { // sta ako je ovo pozvano pre prve sekcije???
      Position *firstP = s->positionsOfUse;
      if (!firstP)
      {
        s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      }
      else
      {
        while (firstP->next)
        {
          firstP = firstP->next;
        }
        firstP->next = new Position(locationCounter, "PC12", curSection->ndx);
      }
      curSection->code.push_back(0);
      locationCounter += 2;
    }
    else
    { // simbol je definisan i u istoj je sekciji
      long offset = s->value - locationCounter - 2;
      unsigned char c = curSection->code.at(locationCounter);
      curSection->code.pop_back();
      curSection->code.pop_back();
      curSection->code.pop_back();
      curSection->code.push_back(prefix | (ps - 8));
      curSection->code.push_back((arg1->num_value & 0xF) | (15 << 4));
      curSection->code.push_back((offset) & 0xF | c);
      curSection->code.push_back((offset >> 4) & 0xFF);
      locationCounter += 2;
    }
  }
}

void Asembler::createArithmeticInstruction(string name, Argument *arg1, Argument *arg2)
{

  unsigned char prefix = 5;
  prefix <<= 4;
  switch (stringToEnum(name))
  {
  case ADD:

    curSection->code.push_back(prefix);
    break;

  case SUB:

    curSection->code.push_back(prefix | 1);
    break;

  case MUL:

    curSection->code.push_back(prefix | 2);
    break;

  case DIV:

    curSection->code.push_back(prefix | 3);
    break;

  default:
    cout << "Unrecognized arithmetic operation\n";
    free(arg1);
    free(arg2);
    exit(-1);
  }

  curSection->code.push_back((arg2->num_value << 4) | (arg2->num_value & 0xF));
  curSection->code.push_back(arg1->num_value << 4);
  curSection->code.push_back(0);

  locationCounter += 4;

  free(arg1);
  free(arg2);
}

void Asembler::createLogicalInstruction(string name, Argument *arg1, Argument *arg2)
{

  unsigned char prefix = 6;
  prefix <<= 4;

  switch (stringToEnum(name))
  {
  case NOT:

    curSection->code.push_back(prefix);
    curSection->code.push_back((arg1->num_value << 4) | (arg1->num_value & 0xF));
    curSection->code.push_back(0);
    curSection->code.push_back(0);

    break;

  case AND:

    curSection->code.push_back(prefix | 1);
    curSection->code.push_back((arg2->num_value << 4) | (arg2->num_value & 0xF));
    curSection->code.push_back(arg1->num_value << 4);
    curSection->code.push_back(0);

    break;

  case OR:

    curSection->code.push_back(prefix | 2);
    curSection->code.push_back((arg2->num_value << 4) | (arg2->num_value & 0xF));
    curSection->code.push_back(arg1->num_value << 4);
    curSection->code.push_back(0);

    break;

  case XOR:

    curSection->code.push_back(prefix | 3);
    curSection->code.push_back((arg2->num_value << 4) | (arg2->num_value & 0xF));
    curSection->code.push_back(arg1->num_value << 4);
    curSection->code.push_back(0);

    break;

  default:
    cout << "Unrecognized logical operation\n";
    free(arg1);
    free(arg2);
    exit(-1);
  }
  locationCounter += 4;

  free(arg1);
  free(arg2);
}

void Asembler::createShiftInstruction(string name, Argument *arg1, Argument *arg2)
{
  unsigned char prefix = 7;
  prefix <<= 4;

  switch (stringToEnum(name))
  {
  case SHL:

    curSection->code.push_back(prefix);
    break;

  case SHR:

    curSection->code.push_back(prefix | 1);
    break;

  default:
    cout << "Unrecognized shift instruction\n";
    free(arg1);
    free(arg2);
    exit(-1);
  }
  locationCounter += 4;

  curSection->code.push_back((arg2->num_value << 4) | (arg2->num_value & 0xF));
  curSection->code.push_back(arg1->num_value << 4);
  curSection->code.push_back(0);

  free(arg1);
  free(arg2);
}

void Asembler::createXchgInstruction(Argument *arg1, Argument *arg2)
{
  unsigned char prefix = 4;
  prefix <<= 4;

  curSection->code.push_back(prefix);
  curSection->code.push_back(arg1->num_value & 0xF);
  curSection->code.push_back(arg2->num_value << 4);
  curSection->code.push_back(0);

  locationCounter += 4;

  free(arg1);
  free(arg2);
}

void Asembler::createLdInstruction(Argument *arg1, Argument *arg2, Argument *arg3)
{

  unsigned char prefix = 9;
  prefix <<= 4;
  string type = arg1->type;

  if (type == "$num")
  {
    string t;
    if (arg1->num_value < -2048 || arg1->num_value > 2047)
    {
      curSection->code.push_back(prefix | 2);
      t = "PC12";
      curSection->code.push_back((arg3->num_value << 4) & 0xFF | 15);
    }
    else
    {
      curSection->code.push_back(prefix | 1);
      t = "12";
      curSection->code.push_back((arg3->num_value << 4) & 0xFF);
    }
    locationCounter += 2;
    Literal *l = curSection->existsInLiteralTable(arg1->num_value);
    if (!l)
    {
      curSection->addLiteral(new Literal(arg1->num_value, sizeof(arg1->num_value), new Position(locationCounter, t, curSection->ndx)));
    }
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, t, curSection->ndx);
    }
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "$symb")
  {
    curSection->code.push_back(prefix | 2);
    curSection->code.push_back((arg3->num_value << 4) & 0xFF | 15);
    locationCounter += 2;
    Symbol *s = existsInSymbolTable(arg1->symb_value);
    if (!s)
    {
      s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg1->symb_value);
      s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      tableOfSymbols.push_back(s);
    }
    else
    {
      Position *first = s->positionsOfUse;
      if (!first)
        s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      else
      {
        while (first->next)
        {
          first = first->next;
        }
        first->next = new Position(locationCounter, "PC12", curSection->ndx);
      }
    }
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "num")
  {
    // citanje sa adrese num
    // najpre upisemo u registar vrednost num
    arg1->type = "$num";
    createLdInstruction(new Argument(arg1->type, arg1->num_value, arg1->symb_value), nullptr, new Argument(arg3->type, arg3->num_value, arg3->symb_value)); // ovo ce zavrsiti u prvoj grani i upisati u registar arg3 vrednost literala arg1
    arg3->type = "[reg]";
    createLdInstruction(new Argument(arg3->type, arg3->num_value, arg3->symb_value), nullptr, new Argument(arg3->type, arg3->num_value, arg3->symb_value)); // ovo ce zavrsiti u poslednjoj grani i upisati u registar arg3 vrednost sa adrese sadrzane u arg3
  }
  else if (type == "symb")
  {
    // citanje sa adrese symb
    arg1->type = "$symb";
    createLdInstruction(new Argument(arg1->type, arg1->num_value, arg1->symb_value), nullptr, new Argument(arg3->type, arg3->num_value, arg3->symb_value)); // ovo ce zavrsisti u drugoj grani i upisati u registar arg3 vrednost simbola arg1
    arg3->type = "[reg]";
    createLdInstruction(new Argument(arg3->type, arg3->num_value, arg3->symb_value), nullptr, new Argument(arg3->type, arg3->num_value, arg3->symb_value)); // ovo ce zavrsiti u poslednjoj grani i upisati u registar arg3 vrednost sa adrese sadrzane u arg3
  }
  else if (type == "reg" && arg2 != nullptr && arg2->type == "num")
  {

    if (arg2->num_value > 2047 || arg2->num_value < -2048)
    {
      cout << "AS ERROR (" << inputFile << "): Given literal for ld instruction is to big\n";
      free(arg1);
      free(arg2);
      free(arg3);
      exit(-1);
    }

    curSection->code.push_back(prefix | 2);
    curSection->code.push_back((arg3->num_value << 4) & 0xFF | (arg1->num_value & 0xF));
    locationCounter += 2;
    Literal *l = curSection->existsInLiteralTable(arg2->num_value);
    if (!l)
    {
      curSection->addLiteral(new Literal(arg2->num_value, sizeof(arg2->num_value), new Position(locationCounter, "12", curSection->ndx)));
    }
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, "12", curSection->ndx);
    }
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "reg" && arg2 != nullptr && arg2->type == "symb")
  {

    // potrebno je proveriti velicinu simbola i da li je equ!!!
    /*
      curSection->code.push_back(prefix | 2);
      curSection->code.push_back((arg3->num_value << 4) & 0xFF | (arg1->num_value & 0xF));
      locationCounter += 2;
      Symbol *s = existsInSymbolTable(arg2->symb_value);
      if (!s)
      {
        s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg2->symb_value);
        s->positionsOfUse = new Position(locationCounter, "12REGIND", curSection);
        tableOfSymbols.push_back(s);
      }
      else
      {
        Position *first = s->positionsOfUse;
        if (!first)
          s->positionsOfUse = new Position(locationCounter, "12REGIND", curSection);
        else
        {
          while (first->next)
          {
            first = first->next;
          }
          first->next = new Position(locationCounter, "12REGIND", curSection);
        }
      }
      curSection->code.push_back(0);
      curSection->code.push_back(0);
      locationCounter += 2;
      */
    cout << "AS ERROR (" << inputFile << "): Value of symbol " << arg2->symb_value << " is not known in time of assembling\n";
    exit(-1);
  }
  else if (type == "reg")
  {

    curSection->code.push_back(prefix | 1);
    curSection->code.push_back((arg3->num_value << 4) & 0xFF | (arg1->num_value & 0xF));
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 4;
  }
  else if (type == "[reg]")
  {
    curSection->code.push_back(prefix | 2);
    curSection->code.push_back((arg3->num_value << 4) & 0xFF | (arg1->num_value & 0xF));
    curSection->code.push_back(0);
    curSection->code.push_back(0);
    locationCounter += 4;
  }

  free(arg1);
  free(arg2);
  free(arg3);
}

void Asembler::createStInstruction(Argument *arg1, Argument *arg2, Argument *arg3)
{
  string type = arg2->type;
  unsigned char prefix = 8;
  prefix <<= 4;
  if (type == "$num")
  {
    cout << "AS ERROR (" << inputFile << "):St with immediate operand\n";
    exit(-1);
  }
  else if (type == "$symb")
  {
    cout << "AS ERROR (" << inputFile << "):St with immediate operand\n";
    exit(-1);
  }
  else if (type == "num")
  {
    string t;
    if (arg2->num_value <= 2047 && arg2->num_value >= -2048)
    {
      curSection->code.push_back(prefix);
      curSection->code.push_back(0);
      t = "12";
    }
    else
    {
      curSection->code.push_back(prefix | 2);
      curSection->code.push_back(15);
      t = "PC12";
    }
    curSection->code.push_back((arg1->num_value << 4) & 0xFF);
    locationCounter += 2;
    Literal *l = curSection->existsInLiteralTable(arg2->num_value);
    if (!l)
    {
      curSection->addLiteral(new Literal(arg2->num_value, sizeof(arg2->num_value), new Position(locationCounter, t, curSection->ndx)));
    }
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, t, curSection->ndx);
    }
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "symb")
  {
    curSection->code.push_back(prefix | 2);
    curSection->code.push_back(15);
    locationCounter += 2;
    curSection->code.push_back((arg1->num_value << 4) & 0xFF);
    Symbol *s = existsInSymbolTable(arg2->symb_value);
    if (!s)
    {
      s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg2->symb_value);
      s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      tableOfSymbols.push_back(s);
    }
    else
    {
      Position *first = s->positionsOfUse;
      if (!first)
        s->positionsOfUse = new Position(locationCounter, "PC12", curSection->ndx);
      else
      {
        while (first->next)
        {
          first = first->next;
        }
        first->next = new Position(locationCounter, "PC12", curSection->ndx);
      }
    }
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "reg" && arg3 != nullptr && arg3->type == "num")
  {

    if (arg3->num_value > 2047 || arg3->num_value < -2048)
    {
      cout << "AS ERROR (" << inputFile << "): Given literal for st instruction is to big\n";
      free(arg1);
      free(arg2);
      free(arg3);
      exit(-1);
    }

    curSection->code.push_back(prefix | 2);
    curSection->code.push_back(arg2->num_value);
    locationCounter += 2;
    curSection->code.push_back((arg1->num_value << 4) & 0xFF);
    Literal *l = curSection->existsInLiteralTable(arg3->num_value);
    if (!l)
    {
      curSection->addLiteral(new Literal(arg3->num_value, sizeof(arg3->num_value), new Position(locationCounter, "12", curSection->ndx)));
    }
    else
    {
      Position *first = l->positinsOfUse;
      while (first->next)
      {
        first = first->next;
      }
      first->next = new Position(locationCounter, "12", curSection->ndx);
    }
    curSection->code.push_back(0);
    locationCounter += 2;
  }
  else if (type == "reg" && arg3 != nullptr && arg3->type == "symb")
  {
    // potrebno je proveriti velicinu simbola i da li je equ!!!
    /*
      curSection->code.push_back(prefix | 2);
      curSection->code.push_back(arg2->num_value);
      locationCounter += 2;
      curSection->code.push_back((arg1->num_value << 4) & 0xFF);
      Symbol *s = existsInSymbolTable(arg3->symb_value);
      if (!s)
      {
        s = new Symbol(curNumOfSymbols++, 0, 0, "NOTYP", '?', -1, arg3->symb_value);
        s->positionsOfUse = new Position(locationCounter, "12REGIND", curSection);
        tableOfSymbols.push_back(s);
      }
      else
      {
        Position *first = s->positionsOfUse;
        if (!first)
          s->positionsOfUse = new Position(locationCounter, "12REGIND", curSection);
        else
        {
          while (first->next)
          {
            first = first->next;
          }
          first->next = new Position(locationCounter, "12REGIND", curSection);
        }
      }
      curSection->code.push_back(0);
      locationCounter += 2;
    */
    cout << "AS ERROR (" << inputFile << "): Value of symbol " << arg3->symb_value << " is not known in time of assembling\n";
    exit(-1);
  }
  else if (type == "reg")
  { /*
     curSection->code.push_back(prefix);
     curSection->code.push_back(arg2->num_value);
     curSection->code.push_back((arg1->num_value << 4) & 0xFF);
     curSection->code.push_back(0);
     locationCounter += 4;
     */
    cout << "AS ERROR (" << inputFile << "): St with regdir\n";
    exit(-1);
  }
  else if (type == "[reg]")
  {
    curSection->code.push_back(prefix);
    curSection->code.push_back(arg2->num_value & 0xFF);
    curSection->code.push_back((arg1->num_value << 4) & 0xFF);
    curSection->code.push_back(0);
    locationCounter += 4;
  }

  free(arg1);
  free(arg2);
  free(arg3);
}

void Asembler::createCsrInstruction(string rw, Argument *arg1, Argument *arg2)
{
  unsigned char prefix = 9;
  prefix <<= 4;

  if (rw == "read")
  {
    curSection->code.push_back(prefix);
  }
  else
  { // write
    curSection->code.push_back(prefix | 4);
  }

  curSection->code.push_back((arg2->num_value << 4) & 0xFF | (arg1->num_value & 0xF));
  curSection->code.push_back(0);
  curSection->code.push_back(0);

  locationCounter += 4;
  free(arg1);
  free(arg2);
}

void Asembler::createPushInstruction(Argument *arg1)
{
  curSection->code.push_back((8 << 4) | 1);
  curSection->code.push_back(14 << 4);
  curSection->code.push_back((arg1->num_value << 4) | 12);
  curSection->code.push_back(~0);
  locationCounter += 4;

  free(arg1);
}

void Asembler::createPopInstruction(Argument *arg1, unsigned char pom)
{
  if (arg1->type == "reg")
  {
    curSection->code.push_back(9 << 4 | 3);
  }
  else
  { // csr
    curSection->code.push_back(9 << 4 | 7);
  }

  curSection->code.push_back((arg1->num_value << 4) | 14);
  curSection->code.push_back(pom);
  curSection->code.push_back(0);
  locationCounter += 4;

  free(arg1);
}

void Asembler::createIretInstruction()
{
  // prvo treba dohvatiti status koji je drugi na steku
  curSection->code.push_back((9 << 4) | 6);
  curSection->code.push_back(14);
  curSection->code.push_back(4);
  curSection->code.push_back(0);
  locationCounter += 4;
  // sada treba ucitati vrednost u pc i sp inkrementirati za 8
  createPopInstruction(new Argument("reg", 15, "x"), (unsigned char)8);
}

void Asembler::finishAssembling()
{
  checkSymbols();
  createLiteralPools();
  createRellocationTableForSections();
  createObjectFile();
  createTextFile();

  cout << "Finished assembling of file " << inputFile << "\n";
  exit(0);
}

void Asembler::createTextFile()
{
  string nameOfOutputTxt = outputFile;
  nameOfOutputTxt += ".txt";
  textOutput = ofstream(nameOfOutputTxt);
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
    textOutput << s->name << " " << s->ndx << " " << s->getOffsetInObjectFile() << " " << s->size << endl;
  }

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
}

Symbol *Asembler::existsInSymbolTable(string s)
{
  for (int i = 0; i < tableOfSymbols.size(); i++)
  {
    if (tableOfSymbols.at(i)->name == s)
      return tableOfSymbols.at(i);
  }
  return nullptr;
}

void Asembler::createLiteralPools()
{
  for (auto s : tableOfSections)
  {
    locationCounter = s->size;
    for (int i = 0; i < s->tableOfLiterals.size(); i++)
    {
      Literal *l = s->tableOfLiterals[i];
      int x = l->num;
      if ((l->num) < -2048 || (l->num) > 2047)
      {
        for (int i = 0; i < 4; i++)
        {
          s->code.push_back(x);
          x >>= 8;
        }
      }
      for (Position *p = l->positinsOfUse; p; p = p->next)
      {
        x = l->num;
        string type = p->typeOfUse;
        if (type == "12")
        {
          s->code[p->loc] = s->code[p->loc] | x & 0xF;
          s->code[p->loc + 1] = x >> 4;
        }
        else
        { //"PC12"

          long offset = locationCounter - p->loc - 2;
          s->code[p->loc] = s->code[p->loc] | (offset & 0xF);
          s->code[p->loc + 1] = offset >> 4;
        }
      }
      if ((l->num) < -2048 || (l->num) > 2047)
        locationCounter += 4;
    }
    s->size = locationCounter;
  }
}

void Asembler::createRellocationTableForSections()
{

  bool poolCreated;     // da li je za dati simbol vec alociran prostor u bazenu
  long posistionInPool; // ako jeste gde

  for (auto s : tableOfSymbols)
  {
    poolCreated = false;
    Position *first = s->positionsOfUse;
    while (first)
    {
      Section *sec = getSectionWithNdx(first->secNdx);
      if (first->typeOfUse == "PC12")
      {
        // alociranje prostora u bazenu
        if (!poolCreated)
        {
          locationCounter = sec->size;
          sec->code.push_back(0);
          sec->code.push_back(0);
          sec->code.push_back(0);
          sec->code.push_back(0);
          posistionInPool = locationCounter;
          sec->size += 4;
          poolCreated = true;

          long symbol;
          long addend;
          if (s->bind == 'g')
          {
            symbol = s->num;
            addend = 0;
          }
          else
          {
            symbol = s->ndx;
            addend = s->value;
          }
          sec->addRellocation(new Rellocation(locationCounter, "32X", symbol, addend));
        }
        else
        {
          locationCounter = posistionInPool;
        }

        int offset = locationCounter - first->loc - 2;

        sec->code[first->loc] = sec->code[first->loc] | (offset & 0xF);
        sec->code[first->loc + 1] = sec->code[first->loc + 1] | (offset >> 4);
      }
      else
      { // 32
        int symbol;
        int addend;
        if (s->bind == 'g')
        {
          symbol = s->num;
          addend = 0;
        }
        else
        {
          symbol = s->ndx;
          addend = s->value;
        }
        sec->addRellocation(new Rellocation(first->loc, "32X", symbol, addend));
      }

      first = first->next;
    }
  }
}

Section *Asembler::getSectionWithNdx(int ndx)
{
  for (auto s : tableOfSections)
  {
    if (s->ndx == ndx)
      return s;
  }
  return nullptr;
}

void Asembler::checkSymbols()
{
  for (auto s : tableOfSymbols)
  {
    if (s->ndx == -1) // .global used for .extern
    {                 /*
                       cout << "AS ERROR (" << inputFile << "): Symbol " << s->name << " used but not defined nor marked as extern\n";
                       exit(-1);
                     */
      s->ndx = 0;
    }
  }
}

void Asembler::createObjectFile()
{

  foutput = ofstream(outputFile, ios::out | ios::binary);
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
