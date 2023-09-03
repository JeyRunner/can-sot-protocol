#include <lyra/lyra.hpp>
#include "CliArgs.h"
#include "GenDefFile.h"
#include "GenCode.h"

using namespace lyra;
using namespace std;


int main(int argc, const char **argv) {
  // cli args
  CliArgs cliArgs;
  cliArgs.parse(argc, argv);
  std::vector<std::string> argList;
  for(int i=0; i<argc; i++) {
      argList.emplace_back(argv[i]);
  }
  // replace executable name
  argList[0] = "canSotCli";

  if (cliArgs.genProtocolSpecCommand.doCommand) {
    GenDefFile genDefFile(cliArgs.genProtocolSpecCommand);
    genDefFile.genDefFile();
  }
  else if (cliArgs.genProtocolCodeCommand.doCommand) {
      GenCode genCode(cliArgs.genProtocolCodeCommand);
      genCode.genCode(argList);
  }
  else {
    cout << "Error: no command to execute specified." << endl << endl;
    cout << cliArgs.cli << endl;
    exit(1);
  }
}