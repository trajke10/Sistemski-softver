#include "../inc/Emulator.hpp"
#include <regex>

void ioJob()
{

  struct termios old_settings, new_settings;

  // Get the current terminal settings
  if (tcgetattr(STDIN_FILENO, &old_settings) != 0)
  {
    perror("tcgetattr");
    return;
  }

  // Copy the current settings to the new_settings
  new_settings = old_settings;

  // Modify the new settings
  new_settings.c_lflag &= ~(ICANON | ECHO); // Disable canonical mode and echo

  new_settings.c_cc[VTIME] = 0;

  new_settings.c_cc[VMIN] = 0;

  // Set the new settings
  if (tcsetattr(STDIN_FILENO, TCSANOW, &new_settings) != 0)
  {
    perror("tcsetattr");
    return;
  }

  char cha;
  while (!Emulator::getEmulator().end)
  {
    if (read(STDIN_FILENO, &cha, 1) == 1)
    {
      Emulator::getEmulator().generateIntIn(cha);
    }
  }

   // Restore the original terminal settings
  if (tcsetattr(STDIN_FILENO, TCSANOW, &old_settings) != 0)
  {
    perror("tcsetattr");
    return;
  }
}

int main(int argc, char *argv[])
{

  regex pattern(R"--(([a-zA-Z0-9_]+).hex)--");
  smatch match;

  if (argc > 2)
  {
    cout << "To many arguments for emulator\n";
    exit(-1);
  }
  if (argc == 1)
  {
    cout << "No hex file specified\n";
    exit(-1);
  }

  string input = argv[1];
  if (!regex_search(input, match, pattern))
  {
    cout << "File should have .hex extension\n";
    exit(-1);
  }

  thread iothread(ioJob);

  Emulator::getEmulator().initialize(input);

  Emulator::getEmulator().startEmulation();

  iothread.join();

  return 0;
}