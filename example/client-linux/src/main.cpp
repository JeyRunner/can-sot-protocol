#include <lyra/lyra.hpp>
#include <communication/SOTClient.h>
#include <thread>
#include "protocol_generated/SOTProtocol.hpp"
#include <linux_socketCan/SocketCanInterface.hpp>

using namespace lyra;
using namespace std;


// cli args
string args_canInterface = "can0";
unsigned int args_clientDeviceId = 1;


int main(int argc, const char **argv) {
  // cli args
  auto cli = lyra::cli();
  bool showHelp = false;
  cli.add_argument(help(showHelp));
  cli.add_argument(lyra::opt(args_canInterface, "can-interface" )["-i"]["--can-interface"]("The can interface to connect to"));
  cli.add_argument(lyra::opt(args_clientDeviceId, "client-device-id" )["-d"]["--client-device-id"]("The sot device id of this client"));
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
  SocketCanInterface canInterface(args_canInterface, args_clientDeviceId);
  if (!canInterface.startCanInterface()) {
    exit(1);
  }
  SOTClient<TestProtocol, CanInterface> sotClient(canInterface, args_clientDeviceId);

  // set some values
  sotClient.getProtocol().objectTree._meta.protocolVersion.write(11);
  sotClient.getProtocol().objectTree.settings.value1.write(5);
  sotClient.getProtocol().objectTree.settings.value2.write(10);
  sotClient.getProtocol().objectTree.settings.subSettings.value3.write(0.25);


  // main communication loop
  while (true) {
    // handle all received can frames
    sotClient.processCanFrames();

    // on first connected
    if (sotClient.gotConnectedEvent.checkAndReset()) {
      cout << "got connected to master" << endl;
    }


    // when received new value from remote client
    if (sotClient.getProtocol().objectTree.settings.subSettings.value3.receivedValueUpdate.checkAndReset()) {
      cout << "got new value from client: settings.subSettings.value3 = " << sotClient.getProtocol().objectTree.settings.subSettings.value3.read() << endl;
    }

    // change some value
    sotClient.getProtocol().objectTree.settings.subSettings.value3.write(
            sotClient.getProtocol().objectTree.settings.subSettings.value3.read()+0.001
            );


    // wait
    this_thread::sleep_for(1ms);
  }
}