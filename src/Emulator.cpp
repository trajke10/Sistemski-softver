#include "../inc/Emulator.hpp"

Emulator &Emulator::getEmulator()
{
  static Emulator instance;
  if (!instance.flag)
  {
    instance.flag = true;
    instance.m.memory = (unsigned char *)mmap(nullptr, 4294967296, PROT_WRITE, MAP_ANONYMOUS | MAP_NORESERVE | MAP_PRIVATE, -1, 0);
    instance.m.illegalInstruction = false;
    instance.m.softwareInt = false;
    instance.m.terminalIn = false;
  }
  return instance;
}

void Emulator::initialize(string input)
{

  m.gpr[15] = m.startAddress;
  m.gpr[14] = m.stackStartAddress;

  inputFile = ifstream(input, ios::binary | ios::in);

  if (!inputFile.is_open())
  {
    cout << "Unable to open file " << input << endl;
    exit(-1);
  }

  string line;

  while (!inputFile.eof())
  {
    string address;
    address.resize(10);
    inputFile.read((char *)(address.c_str()), 10);
    if (!isxdigit(address[0]))
      continue;
    address = "0x" + address.substr(0, 8);
    unsigned long position = stol(address, 0, 16);
    for (long i = 0; i < 8; i++)
    {
      unsigned char c;
      inputFile.read((char *)&c, 1);
      m.memory[position++] = c;
    }
    char garbage[1];
    inputFile.read(garbage, 1); //\n
  }
}

void Emulator::startEmulation()
{

  while (!end)
  {
    readInstruction();

    checkForInterrupts();
  }

  endEmulation();
}

void Emulator::readInstruction()
{

  unsigned char instr = m.memory[m.gpr[15]];

  m.gpr[15]++;

  unsigned int pom = (unsigned int)instr;

  unsigned int opCode = (pom >> 4) & 0xF;

  unsigned int mod = pom & 0xF;

  unsigned int a;

  unsigned int b;

  unsigned int c;

  int d;

  if (opCode == 0)
  { // halt
    // cout << "HALT" << endl;
    m.gpr[15] += 3;
    end = true;
  }
  else if (opCode == 1)
  { // int
    // cout << "INT" << endl;

    m.gpr[15] += 3;

    m.softwareInt = true;
  }
  else if (opCode == 2)
  { // call

    // cout << "CALL" << endl;

    getOperands(&a, &b, &c, &d);

    // push pc
    m.gpr[14]--;
    m.memory[m.gpr[14]] = (m.gpr[15] >> 24) & 0xFF;
    m.gpr[14]--;
    m.memory[m.gpr[14]] = (m.gpr[15] >> 16) & 0xFF;
    m.gpr[14]--;
    m.memory[m.gpr[14]] = (m.gpr[15] >> 8) & 0xFF;
    m.gpr[14]--;
    m.memory[m.gpr[14]] = (m.gpr[15]) & 0xFF;

    if (mod == 0)
    {
      m.gpr[15] = m.gpr[a] + m.gpr[b] + d;
    }
    else if (mod == 1)
    {
      unsigned long x = m.gpr[a] + m.gpr[b] + d;

      m.gpr[15] = m.memory[x];

      m.gpr[15] = m.gpr[15] | (m.memory[x + 1] << 8);

      m.gpr[15] = m.gpr[15] | (m.memory[x + 2] << 16);

      m.gpr[15] = m.gpr[15] | (m.memory[x + 3] << 24);
    }
    else
    { /*
       cout << "Unrecognized instruction\n";
       exit(-1);
       */
      m.illegalInstruction = true;
    }
  }
  else if (opCode == 3)
  { // jmp or branch

    // cout << "JMP" << endl;

    getOperands(&a, &b, &c, &d);

    if (mod == 0)
    {
      m.gpr[15] = m.gpr[a] + d;
    }
    else if (mod == 1)
    {
      if (m.gpr[b] == m.gpr[c])
      {
        m.gpr[15] = m.gpr[a] + d;
      }
    }
    else if (mod == 2)
    {
      if (m.gpr[b] != m.gpr[c])
      {
        m.gpr[15] = m.gpr[a] + d;
      }
    }
    else if (mod == 3)
    {
      if (m.gpr[b] > m.gpr[c])
      {
        m.gpr[15] = m.gpr[a] + d;
      }
    }
    else if (mod == 8)
    {
      unsigned long x = m.gpr[a] + d;

      m.gpr[15] = m.memory[x];

      m.gpr[15] = m.gpr[15] | (m.memory[x + 1] << 8);

      m.gpr[15] = m.gpr[15] | (m.memory[x + 2] << 16);

      m.gpr[15] = m.gpr[15] | (m.memory[x + 3] << 24);
    }
    else if (mod == 9)
    {
      if (m.gpr[b] == m.gpr[c])
      {
        unsigned long x = m.gpr[a] + d;

        m.gpr[15] = m.memory[x];

        m.gpr[15] = m.gpr[15] | (m.memory[x + 1] << 8);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 2] << 16);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 3] << 24);
      }
    }
    else if (mod == 10)
    {
      if (m.gpr[b] != m.gpr[c])
      {
        unsigned long x = m.gpr[a] + d;

        m.gpr[15] = m.memory[x];

        m.gpr[15] = m.gpr[15] | (m.memory[x + 1] << 8);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 2] << 16);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 3] << 24);
      }
    }
    else if (mod == 11)
    {
      if (m.gpr[b] > m.gpr[c])
      {
        int x = m.gpr[a] + d;

        m.gpr[15] = m.memory[x];

        m.gpr[15] = m.gpr[15] | (m.memory[x + 1] << 8);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 2] << 16);

        m.gpr[15] = m.gpr[15] | (m.memory[x + 3] << 24);
      }
    }
    else
    {
      /*
      cout << "Unrecognized instruction\n";
      exit(-1);
      */
      m.illegalInstruction = true;
    }
  }
  else if (opCode == 4)
  { // xchg

    // cout << "XCHG" << endl;
    getOperands(&a, &b, &c, &d);

    int temp = m.gpr[b];
    m.gpr[b] = m.gpr[c];
    m.gpr[c] = temp;

    if (b == 0)
    {
      m.gpr[b] == 0;
    }
    if (c == 0)
    {
      m.gpr[c] == 0;
    }
  }
  else if (opCode == 5)
  { // arithmetic

    // cout << "ARITHEMTIC" << endl;
    getOperands(&a, &b, &c, &d);

    if (mod == 0)
    {
      m.gpr[a] = m.gpr[b] + m.gpr[c];
    }
    else if (mod == 1)
    {
      m.gpr[a] = m.gpr[b] - m.gpr[c];
    }
    else if (mod == 2)
    {
      m.gpr[a] = m.gpr[b] * m.gpr[c];
    }
    else if (mod == 3)
    {
      if (m.gpr[c] == 0)
      {
        /*
      cout << "Division with zero\n";
      exit(-1);
      */
        m.illegalInstruction = true;
      }
      else
        m.gpr[a] = m.gpr[b] / m.gpr[c];
    }
    else
    {
      /*
       cout << "Unrecognized instruction\n";
       exit(-1);
       */
      m.illegalInstruction = true;
    }

    if (a == 0)
    {
      m.gpr[a] = 0;
    }
  }
  else if (opCode == 6)
  { // logical

    // cout << "LOGICAL" << endl;
    getOperands(&a, &b, &c, &d);

    if (mod == 0)
    {
      m.gpr[a] = ~m.gpr[b];
    }
    else if (mod == 1)
    {
      m.gpr[a] = m.gpr[b] & m.gpr[c];
    }
    else if (mod == 2)
    {
      m.gpr[a] = m.gpr[b] | m.gpr[c];
    }
    else if (mod == 3)
    {
      m.gpr[a] = m.gpr[b] ^ m.gpr[c];
    }
    else
    {
      /*
      cout << "Unrecognized instruction\n";
      exit(-1);
      */
      m.illegalInstruction = true;
    }

    if (a == 0)
    {
      m.gpr[a] = 0;
    }
  }
  else if (opCode == 7)
  { // shift

    // cout << "SHIFT" << endl;
    getOperands(&a, &b, &c, &d);

    if (mod == 0)
    {
      m.gpr[a] = m.gpr[b] << m.gpr[c];
    }
    else if (mod == 1)
    {
      m.gpr[a] = m.gpr[b] >> m.gpr[c];
    }
    else
    {
      /*
      cout << "Unrecognized instruction\n";
      exit(-1);
      */
      m.illegalInstruction = true;
    }

    if (a == 0)
    {
      m.gpr[a] = 0;
    }
  }
  else if (opCode == 8)
  { // st

    // cout << "ST" << endl;
    getOperands(&a, &b, &c, &d);

    if (mod == 0)
    {
      unsigned long x = m.gpr[a] + m.gpr[b] + d;

      m.memory[x] = m.gpr[c];
      m.memory[x + 1] = m.gpr[c] >> 8;
      m.memory[x + 2] = m.gpr[c] >> 16;
      m.memory[x + 3] = m.gpr[c] >> 24;
    }
    else if (mod == 1)
    {
      m.gpr[a] = m.gpr[a] + d;

      unsigned long x = m.gpr[a];

      m.memory[x] = m.gpr[c];
      m.memory[x + 1] = m.gpr[c] >> 8;
      m.memory[x + 2] = m.gpr[c] >> 16;
      m.memory[x + 3] = m.gpr[c] >> 24;
    }
    else if (mod == 2)
    {
      unsigned long x = m.gpr[a] + m.gpr[b] + d;

      unsigned long y = m.memory[x];

      y = y | (m.memory[x + 1] << 8);
      y = y | (m.memory[x + 2] << 16);
      y = y | (m.memory[x + 3] << 24);

      y = y & 0x00000000FFFFFFFF;

      m.memory[y] = m.gpr[c];
      m.memory[y + 1] = m.gpr[c] >> 8;
      m.memory[y + 2] = m.gpr[c] >> 16;
      m.memory[y + 3] = m.gpr[c] >> 24;

      if (y == 0x00000000FFFFFF00 || c == 2)
      {
        cout << hex << (char)(m.gpr[c] & 0xFF) << flush;
      }
    }
    else
    {
      /*
      cout << "Unrecognized instruction\n";
      exit(-1);
      */
      m.illegalInstruction = true;
    }
  }
  else if (opCode == 9)
  { // ld

    // cout << "LD" << endl;
    getOperands(&a, &b, &c, &d);
    if (mod == 0)
    {
      if (b > 2)
      {
        /*
      cout << "Wrong index for csr\n";
      exit(-1);
      */
        m.illegalInstruction = true;
      }
      else
      {
        m.gpr[a] = m.csr[b];

        if (a == 0)
        {
          m.gpr[a] = 0;
        }
      }
    }
    else if (mod == 1)
    {
      m.gpr[a] = m.gpr[b] + d;

      if (a == 0)
      {
        m.gpr[a] = 0;
      }
    }
    else if (mod == 2)
    {

      unsigned long x = m.gpr[b] + m.gpr[c] + d;

      m.gpr[a] = m.memory[x];

      m.gpr[a] = m.gpr[a] | (m.memory[x + 1] << 8);

      m.gpr[a] = m.gpr[a] | (m.memory[x + 2] << 16);

      m.gpr[a] = m.gpr[a] | (m.memory[x + 3] << 24);

      if (a == 0)
      {
        m.gpr[a] = 0;
      }
    }
    else if (mod == 3)
    {
      unsigned long x = m.gpr[b];

      m.gpr[a] = m.memory[x];

      m.gpr[a] = m.gpr[a] | (m.memory[x + 1] << 8);

      m.gpr[a] = m.gpr[a] | (m.memory[x + 2] << 16);

      m.gpr[a] = m.gpr[a] | (m.memory[x + 3] << 24);

      m.gpr[b] = m.gpr[b] + d;

      if (a == 0)
      {
        m.gpr[a] = 0;
      }
    }
    else if (mod == 4)
    {
      if (a > 2)
      {
        /*
      cout << "Wrong index for csr\n";
      exit(-1);
      */
        m.illegalInstruction = true;
      }
      else
      {
        m.csr[a] = m.gpr[b];
      }
    }
    else if (mod == 5)
    {
      if (a > 2 || b > 2)
      {
        /*
      cout << "Wrong index for csr\n";
      exit(-1);
      */
        m.illegalInstruction = true;
      }
      else
        m.csr[a] = m.csr[b];
    }
    else if (mod == 6)
    {
      if (a > 2)
      {
        /*
       cout << "Wrong index for csr\n";
       exit(-1);
       */
        m.illegalInstruction = true;
      }
      else
      {

        unsigned long x = m.gpr[b] + m.gpr[c] + d;

        m.csr[a] = m.memory[x];

        m.csr[a] = m.csr[a] | (m.memory[x + 1] << 8);

        m.csr[a] = m.csr[a] | (m.memory[x + 2] << 16);

        m.csr[a] = m.csr[a] | (m.memory[x + 3] << 24);
      }
    }
    else if (mod == 7)
    {
      if (a > 2)
      {
        /*
      cout << "Wrong index for csr\n";
      exit(-1);
      */
        m.illegalInstruction = true;
      }
      else
      {

        long unsigned x = m.gpr[b];

        m.csr[a] = m.memory[x];

        m.csr[a] = m.csr[a] | (m.memory[x + 1] << 8);

        m.csr[a] = m.csr[a] | (m.memory[x + 2] << 16);

        m.csr[a] = m.csr[a] | (m.memory[x + 3] << 24);

        m.csr[b] += d;
      }
    }
  }
  else
  {
    /*
      cout << "Wrong index for csr\n";
      exit(-1);
      */
    m.illegalInstruction = true;
  }
}

void Emulator::getOperands(unsigned int *a, unsigned int *b, unsigned int *c, int *d)
{

  unsigned int ab = (unsigned int)m.memory[m.gpr[15]];
  m.gpr[15]++;
  unsigned int cd = (unsigned int)m.memory[m.gpr[15]];
  m.gpr[15]++;
  unsigned int dd = (unsigned int)m.memory[m.gpr[15]];
  m.gpr[15]++;

  *a = (ab >> 4) & 0xF;
  *b = ab & 0xF;
  *c = (cd >> 4) & 0xF;
  *d = (dd << 4) | (cd & 0xF);
  *d = (*d) << 20;
  *d = (*d) >> 20;
}

void Emulator::generateIntIn(char c)
{
  characterIn = c;

  m.memory[m.term_in] = characterIn;
  m.memory[m.term_in + 1] = characterIn >> 8;
  m.memory[m.term_in + 2] = characterIn >> 16;
  m.memory[m.term_in + 3] = characterIn >> 24;

  m.terminalIn = true;
}

void Emulator::checkForInterrupts()
{
  int type = 0;
  if (m.csr[STATUS] & 0x4)
  { // prekidi su maskirani
    return;
  }
  if (m.illegalInstruction)
  {
    type = 1;
    m.illegalInstruction = false;
  }
  else if (m.terminalIn)
  {
    type = 3;
    m.terminalIn = false;
  }
  else if (m.softwareInt)
  {
    type = 4;
    m.softwareInt = false;
  }

  if (type != 0)
  {
    generateInterrupt(type);
  }
}

void Emulator::generateInterrupt(int type)
{
  // push status
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.csr[STATUS] >> 24) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.csr[STATUS] >> 16) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.csr[STATUS] >> 8) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.csr[STATUS]) & 0xFF;

  // push pc
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.gpr[15] >> 24) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.gpr[15] >> 16) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.gpr[15] >> 8) & 0xFF;
  m.gpr[14]--;
  m.memory[m.gpr[14]] = (m.gpr[15]) & 0xFF;

  m.csr[CAUSE] = type;

  m.csr[STATUS] = m.csr[STATUS] & (~0x1);

  m.gpr[15] = m.csr[HANDLER];
}

void Emulator::print()
{

  unsigned int x;

  x = m.memory[m.term_out];
  x = x | (m.memory[m.term_out + 1] << 8);
  x = x | (m.memory[m.term_out + 2] << 16);
  x = x | (m.memory[m.term_out + 3] << 24);
}

void Emulator::endEmulation()
{
  cout << "\n---------------------------------------------------------------------\n";
  cout << "Emulated processor executed halt instruction\n";
  cout << "Emulated processor state: \n";
  for (int i = 0; i < 16; i++)
  {
    if (i < 10)
      cout << " ";
    cout << "r" << dec << i;
    cout << "=0x" << setw(8) << setfill('0') << hex << m.gpr[i] << "    ";
    if ((i + 1) % 4 == 0)
      cout << endl;
  }
}
