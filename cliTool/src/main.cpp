#include <lyra/lyra.hpp>
#include "CliArgs.h"
#include "GenDefFile.h"

using namespace lyra;
using namespace std;


int main(int argc, const char **argv) {
  // cli args
  CliArgs cliArgs;
  cliArgs.parse(argc, argv);

  if (cliArgs.genProtocolSpecCommand.doCommand) {
    GenDefFile genDefFile(cliArgs.genProtocolSpecCommand);
    genDefFile.genDefFile();
  }
  else if (false) {

  }
  else {
    cout << "Error: no command to executed specified." << endl << endl;
    cout << cliArgs.cli << endl;
    exit(1);
  }
}