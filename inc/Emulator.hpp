#ifndef EMULATOR_HPP
#define EMULATOR_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <sys/mman.h>
#include <iomanip>
#include <termios.h>
#include <thread>
#include <unistd.h>
using namespace std;

#define STATUS 0
#define HANDLER 1
#define CAUSE 2

struct Machine
{
  unsigned char *memory;
  unsigned int gpr[16] = {0};
  unsigned int csr[3] = {0};
  unsigned int startAddress = 0x40000000;
  unsigned int stackStartAddress = 0xFFFFFEFF;
  unsigned int term_in = 0xFFFFFF04;
  unsigned int term_out = 0xFFFFFF00;
  bool terminalIn;
  bool illegalInstruction;
  bool softwareInt;
};

class Emulator
{

private:
  bool flag = false;
  string input;
  ifstream inputFile;
  Machine m;
  unsigned int characterIn;

public:
  bool end;

  static Emulator &getEmulator();

  void initialize(string input);

  void startEmulation();

  void readInstruction();

  void getOperands(unsigned int *a, unsigned int *b, unsigned int *c, int *d);

  void generateIntIn(char c);

  void checkForInterrupts();

  void generateInterrupt(int type);

  void print();

  void endEmulation();
};

#endif