#include <lyra/lyra.hpp>
#include <communication/SOTMaster.h>
#include <thread>
#include "protocol_generated/SOTProtocol.hpp"

using namespace lyra;
using namespace std;


// cli args
string args_canInterface = "can0";
uint8_t  args_clientDeviceId = 1;


int main(int argc, const char **argv) {
  // cli args
  auto cli = lyra::cli();
  bool showHelp = false;
  cli.add_argument(help(showHelp));
  cli.add_argument(lyra::opt(args_canInterface, "can-interface" )["-i"]["--can-interface"]("The can interface to connect to"));
  cli.add_argument(lyra::opt(args_canInterface, "client-device-id" )["-d"]["--client-device-id"]("The sot device id of the client to connect to"));
  auto cli_result = cli.parse({argc, argv});
  if (!cli_result){
    std::cerr << "Error in command line: " << cli_result.message() << std::endl;
    exit(1);
  }
  if (showHelp){
    std::cout << endl << cli << std::endl;
    exit(0);
  }

  cout << "will use can interface '" << args_canInterface << "'" << endl;



  // create master and connect to client
  SOTMaster<TestProtocol> sotMaster;
  sotMaster.addAndConnectToClient(1);
  auto &sotClient = sotMaster.getClient(1);


  // main communication loop
  while (true) {
    // handle all received can frames
    sotMaster.processCanFrames();

    // on first connected
    if (sotMaster.getClient(1).gotConnectedEvent) {
      sotMaster.getClient(1).gotConnectedEvent.clear(); // reset event
      cout << "client with id " << args_clientDeviceId << " got successfully connected" << endl;

      // read some value from client
      sotClient.protocol.sendReadValueReq(sotClient.protocol.objectTree.settings.value1);
    }


    // when received new value from remote client
    if (sotClient.protocol.objectTree.settings.value1.wasChangedEvent) {
      sotClient.protocol.objectTree.settings.value1.wasChangedEvent.clear();
      cout << "got value from client: settings.value1 = " << sotClient.protocol.objectTree.settings.value1.read() << endl;
    }

    // wait
    this_thread::sleep_for(1ms);
  }
}