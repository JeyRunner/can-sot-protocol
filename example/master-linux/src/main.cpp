#include <lyra/lyra.hpp>
#include <communication/SOTMaster.h>
#include <thread>
#include "protocol_generated/SOTProtocol.hpp"
#include <linux_socketCan/SocketCanInterface.hpp>

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
  SocketCanInterface canInterface(args_canInterface);
  if (!canInterface.startCanInterface()) {
    exit(1);
  }

  SOTMaster<TestProtocol, CanInterface> sotMaster(canInterface);
  sotMaster.addAndConnectToClient(args_clientDeviceId);
  auto &sotClient = sotMaster.getClient(args_clientDeviceId);


  // main communication loop
  while (true) {
    auto timeStart = std::chrono::system_clock::now();

    // handle all received can frames
    sotMaster.processCanFrames();

    // on first connected
    if (sotMaster.getClient(args_clientDeviceId).gotConnectedEvent) {
      sotMaster.getClient(args_clientDeviceId).gotConnectedEvent.clear(); // reset event
      cout << "client with id " << (int)args_clientDeviceId << " got successfully connected" << endl;

      // read some value from client
      sotClient.protocol.objectTree.settings.value1.sendReadValueReq();
    }


    // when received new value from remote client
    if (sotClient.protocol.objectTree.settings.value1.receivedValueUpdate.checkAndReset()) {
      cout << "got value from client: settings.value1 = " << sotClient.protocol.objectTree.settings.value1.read() << endl;
    }
    if (sotClient.protocol.objectTree.settings.subSettings.value3.receivedValueUpdate.checkAndReset()) {
      cout << "got value from client: settings.subSettings.value3 = " << sotClient.protocol.objectTree.settings.subSettings.value3.read() << endl;
    }


    // read some value from client
    sotClient.protocol.objectTree.settings.subSettings.value3.sendReadValueReq();


    auto timeEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = timeEnd - timeStart;
    cout << "loop took " << elapsed_seconds.count() << "s" << endl;


    // wait
    this_thread::sleep_for(1ms);
  }
}