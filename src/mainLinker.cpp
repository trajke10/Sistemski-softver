#include "../inc/Linker.hpp"
#include <regex>
#include <string>
using namespace std;

map<string, string> sectionToAddres;

void addToMap(string s, string a)
{
  sectionToAddres.insert(pair<string, string>(s, a));
}

int main(int argc, char *argv[])
{

  regex pattern(R"--(-place=([a-zA-Z0-9_]+)@0x([0-9a-fA-F]{1,}))--");
  smatch match;

  bool hex = false;

  string outputFile;

  bool relocatable = false;

  vector<string> inputFiles;

  for (int i = 1; i < argc; i++)
  {

    string input = argv[i];

    if (input == "-hex")
    {
      hex = true;
    }
    else if (input == "-relocatable")
    {
      relocatable = true;
    }
    else if (input == "-o")
    {
      if (argc < i + 1)
      {
        cout << "ERROR: Name of output file should be after -o\n";
        exit(-1);
      }
      else
      {
        outputFile = argv[i + 1];
        //cout<<outputFile<<endl;
        i++;
      }
    }
    else if (regex_search(input, match, pattern))
    {
      sectionToAddres.insert(pair<string, string>(match[1], match[2]));
    }
    else
    {
      inputFiles.push_back(input);
    }

  }

  if (hex==false && relocatable==false)
  {
    return 0;
  }

  if (hex && relocatable)
  {
    cout << "ERROR: Output can be either hex or relocatable, not both\n";
    exit(-1);
  }

  Linker::getLinker().initialize(inputFiles, hex ? "-hex" : "-relocatable", outputFile, sectionToAddres);

  Linker::getLinker().startLinking();

  return 0;
}