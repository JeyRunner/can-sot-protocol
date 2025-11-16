#include "DocTestIncl.h"
#include <cstring>
#include "communication/SOTMaster.h"
#include "communication/SOTClient.h"
#include "vector"
#include "list"

#include "TestUtil.h"
#include "MockMasterClient.hpp"
#include "communication/threaded/SOTMasterThreaded.h"

using namespace std;


/*
TEST_CASE("MasterThreaded Client communication: addAndConnectToClientBlocking") {
  // not implemented
  CHECK(false);
}
*/

TEST_CASE("MasterThreaded Client communication: init and sendReadValueReqBlocking") {
  TestSOTClient client;

  // start communication thread in background
  SOTMasterThreaded<TestSOTMasterLockable, MockCanBuffer> masterThreaded;
  masterThreaded.startMainThread(2);

  // master thread
  auto masterThread = thread([&] () {
    // lock scope
    //this_thread::sleep_for(10ms);
    cout << "[master] wait for lock ..." << endl;
    auto master = masterThreaded.acquireLock();
    cout << "[master] got lock" << endl;
    master->addAndConnectToClient(1);
    cout << "[master] will call sendReadValueReqBlocking() ..." << endl;
    ConnectedClientLockable<MockTestProtocol, TestSOTMasterLockable> &client = master->getClient(1);
    auto value = client.protocol.objectTree.settings.value1.sendReadValueReqBlocking();
    // @todo value never set -> no end of blocking
    CHECK(value == 72);
    cout << "[master] sendReadValueReqBlocking returned " << value << " -> done" << endl;
  });

  // connect and answer master read req
  // lock scope
  {
    // hacky wait till master send the read request
    this_thread::sleep_for(10ms);
    cout << "[client] waiting for master lock" << endl;
    auto master = masterThreaded.acquireLock();
    client.getProtocol().objectTree.settings.value1.write(72);
    cout << "[client] got lock -> will progress master send packages" << endl;
    // "send" to the client
    client.processCanFramesReceived(master->framesSend);
    // "send" to the master
    master->clearFramesSend();
    master->processCanFramesReceived(client.framesSend);
    client.clearFramesSend();

    master.unlock();
    cout << "[client] released lock" << endl;
  }

  this_thread::sleep_for(5ms);
  cout << "wait to joint ..." << endl;
  masterThreaded.stopMainThread();
  masterThread.join();
}

/*
TEST_CASE("MasterThreaded Client communication: remote call blocking") {
  // not implemented
  CHECK(false);
}
*/