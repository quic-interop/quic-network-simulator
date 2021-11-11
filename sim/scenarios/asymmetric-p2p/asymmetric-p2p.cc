#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/queue.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/queue-size.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "drop-rate-error-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {

    string channelDelay;
    string forwardDataRate;
    string forwardQueueSize;
    string returnDataRate;
    string returnQueueSize;
    string drop_rate_to_client;
    string drop_rate_to_server;

    // for dropping packets:
    random_device rand_dev;
    mt19937 generator(rand_dev());  // Seed random number generator first
    Ptr<DropRateErrorModel> drops_to_client = CreateObject<DropRateErrorModel>();
    Ptr<DropRateErrorModel> drops_to_server = CreateObject<DropRateErrorModel>();

    CommandLine cmd;
    cmd.AddValue("delay", "delay of the channel in both directions (RTT = 2 * delay)", channelDelay);
    cmd.AddValue("forward-data-rate", "data rate of the forward link (right -> left)", forwardDataRate);
    cmd.AddValue("forward-queue", "queue size of the forward link (right -> left) (in packets)", forwardQueueSize);
    cmd.AddValue("return-data-rate", "data rate of the return link (left -> right)", returnDataRate);
    cmd.AddValue("return-queue", "queue size of the return link (left -> right) (in packets)", returnQueueSize);
    cmd.AddValue("drop-rate-to-client", "packet drop rate (towards client)", drop_rate_to_client);
    cmd.AddValue("drop-rate-to-server", "packet drop rate (towards server)", drop_rate_to_server);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(channelDelay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(forwardDataRate.length() == 0, "Missing parameter: forward-data-rate");
    NS_ABORT_MSG_IF(forwardQueueSize.length() == 0, "Missing parameter: forward-queue");
    NS_ABORT_MSG_IF(returnDataRate.length() == 0, "Missing parameter: return-data-rate");
    NS_ABORT_MSG_IF(returnQueueSize.length() == 0, "Missing parameter: return-queue");

    QuicNetworkSimulatorHelper sim;

    QuicPointToPointHelper p2p;
    p2p.SetChannelAttribute("Delay", StringValue(channelDelay));

    NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());

    auto leftP2PNetDev = devices.Get(0)->GetObject<PointToPointNetDevice>();
    auto rightP2PNetDev = devices.Get(1)->GetObject<PointToPointNetDevice>();

    leftP2PNetDev->SetDataRate(DataRate(returnDataRate));
    rightP2PNetDev->SetDataRate(DataRate(forwardDataRate));

    auto returnQueue = leftP2PNetDev->GetQueue();
    auto forwardQueue = rightP2PNetDev->GetQueue();
    returnQueue->SetMaxSize(QueueSize(returnQueueSize + "p"));
    forwardQueue->SetMaxSize(QueueSize(forwardQueueSize + "p"));

    // Set client and server drop rates.
    if (drop_rate_to_client.length() == 0) {
        drops_to_client->SetDropRate(0);
        cout << "Using drop rate to client: 0 %. (Use --drop-rate-to-client to change)" << endl;
    } else {
        drops_to_client->SetDropRate(stoi(drop_rate_to_client));
    }
    if (drop_rate_to_server.length() == 0) {
        drops_to_server->SetDropRate(0);
        cout << "Using drop rate to server: 0 %. (Use --drop-rate-to-server to change)" << endl;
    } else {
        drops_to_server->SetDropRate(stoi(drop_rate_to_server));
    }

    devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(drops_to_client));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(drops_to_server));

    sim.Run(Seconds(36000));
}
