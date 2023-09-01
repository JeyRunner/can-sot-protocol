#include <lyra/lyra.hpp>
#include <communication/SOTMaster.h>
#include <thread>
#include "protocol_generated/SOTProtocol.hpp"
#include <linux_socketCan/SocketCanInterface.hpp>

using namespace lyra;
using namespace std;


// cli args
string args_canInterface = "can0";
unsigned int args_clientDeviceId = 1;
float loopDelayMs = 1;


int main(int argc, const char **argv) {
  // cli args
  auto cli = lyra::cli();
  bool showHelp = false;
  cli.add_argument(help(showHelp));
  cli.add_argument(lyra::opt(args_canInterface, "can-interface" )["-i"]["--can-interface"]("The can interface to connect to"));
  cli.add_argument(lyra::opt(args_clientDeviceId, "client-device-id" )["-d"]["--client-device-id"]("The sot device id of the client to connect to"));
  cli.add_argument(lyra::opt(loopDelayMs, "ms" )["-w"]["--wait-loop-delay"]("The delay in ms for each main loop iteration (between send/receive packages)"));
  auto cli_result = cli.parse({argc, argv});
  if (!cli_result){
    std::cerr << "Error in command line: " << cli_result.errorMessage() << std::endl;
    exit(1);
  }
  if (showHelp){
    std::cout << endl << cli << std::endl;
    exit(0);
  }

  cout << "will use can interface '" << args_canInterface << "'" << endl;



  // create master and connect to client
  SocketCanInterface canInterface(args_canInterface); // own id will be 0 by default
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

    // do nothing as long the client is not connected yet
    if (!sotMaster.getClient(args_clientDeviceId).isConnected()) {
      continue;
    }

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
    if (sotClient.protocol.objectTree.debug.clientRxBufferNumPackages.receivedValueUpdate.checkAndReset()) {
      cout << "got debug.clientRxBufferNumPackages = " << sotClient.protocol.objectTree.debug.clientRxBufferNumPackages.read() << endl;
    }
    if (sotClient.protocol.objectTree.debug.clientTxBufferNumPackages.receivedValueUpdate.checkAndReset()) {
      cout << "got debug.clientTxBufferNumPackages = " << sotClient.protocol.objectTree.debug.clientTxBufferNumPackages.read() << endl;
    }
    if (sotClient.protocol.objectTree.debug.clientProcessPackagesDurationMs.receivedValueUpdate.checkAndReset()) {
      cout << "got debug.clientProcessPackagesDurationMs = " << sotClient.protocol.objectTree.debug.clientProcessPackagesDurationMs.read() << endl;
    }

    // check for errors
    if (sotClient.onCommunicationErrorRxOverflow.checkAndReset()) {
      cerr << "!!!!!! got communication error from client:  RxOverflow !!!!!!!" << endl;
    }
    if (sotClient.onCommunicationErrorTxOverflow.checkAndReset()) {
      cerr << "!!!!!! got communication error from client:  TxOverflow !!!!!!!" << endl;
    }


    // read some value from client
    sotClient.protocol.objectTree.settings.subSettings.value3.sendReadValueReq();
    sotClient.protocol.objectTree.debug.clientRxBufferNumPackages.sendReadValueReq();
    sotClient.protocol.objectTree.debug.clientTxBufferNumPackages.sendReadValueReq();
    sotClient.protocol.objectTree.debug.clientProcessPackagesDurationMs.sendReadValueReq();




    auto timeEnd = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = timeEnd - timeStart;
    cout << "loop took " << elapsed_seconds.count() << "s" << endl;


    // wait
    int waitUs = (unsigned int) (loopDelayMs * 1000.0);
    this_thread::sleep_for(std::chrono::microseconds(waitUs));
    //cout << waitUs << endl;
  }
}