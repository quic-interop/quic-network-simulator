#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/queue.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/queue-size.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {

    std::string channelDelay;
    std::string forwardDataRate;
    std::string forwardQueueSize;
    std::string returnDataRate;
    std::string returnQueueSize;

    CommandLine cmd;
    cmd.AddValue("delay", "delay of the channel in both directions (RTT = 2 * delay)", channelDelay);
    cmd.AddValue("forward-data-rate", "data rate of the forward link (right -> left)", forwardDataRate);
    cmd.AddValue("forward-queue", "queue size of the forward link (right -> left) (in packets)", forwardQueueSize);
    cmd.AddValue("return-data-rate", "data rate of the return link (left -> right)", returnDataRate);
    cmd.AddValue("return-queue", "queue size of the return link (left -> right) (in packets)", returnQueueSize);
    cmd.Parse(argc, argv);

    NS_ABORT_MSG_IF(channelDelay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(forwardDataRate.length() == 0, "Missing parameter: forward-data-rate");
    NS_ABORT_MSG_IF(forwardQueueSize.length() == 0, "Missing parameter: forward-queue");
    NS_ABORT_MSG_IF(returnDataRate.length() == 0, "Missing parameter: return-data-rate");
    NS_ABORT_MSG_IF(returnQueueSize.length() == 0, "Missing parameter: return-queue");

    QuicNetworkSimulatorHelper sim;

    // Stick in the point-to-point line between the sides.
    // QuicPointToPointHelper p2pLeft, p2pRight;
    // p2pLeft.SetDeviceAttribute("DataRate", StringValue(returnDataRate));
    // p2pLeft.SetChannelAttribute("Delay", StringValue(returnDelay));
    // p2pLeft.SetQueueSize(StringValue(returnQueue + "p"));
    // p2pRight.SetDeviceAttribute("DataRate", StringValue(forwardDataRate));
    // p2pRight.SetChannelAttribute("Delay", StringValue(forwardDelay));
    // p2pRight.SetQueueSize(StringValue(forwardQueue + "p"));
    //
    // NetDeviceContainer devices;
    // devices.Add(p2pLeft.Install(sim.GetLeftNode(), p2pRight));
    // devices.Add(p2pRight.Install(p2pLeft, sim.GetRightNode()));

    QuicPointToPointHelper p2p;
    p2p.SetChannelAttribute("Delay", StringValue(channelDelay));
    // p2p.SetDeviceAttribute("DataRate", StringValue(forwardDataRate));
    // p2p.SetQueueSize(StringValue(forwardQueue + "p"));

    NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());

    auto leftP2PNetDev = devices.Get(0)->GetObject<PointToPointNetDevice>();
    auto rightP2PNetDev = devices.Get(1)->GetObject<PointToPointNetDevice>();

    leftP2PNetDev->SetDataRate(DataRate(returnDataRate));
    rightP2PNetDev->SetDataRate(DataRate(forwardDataRate));

    auto returnQueue = leftP2PNetDev->GetQueue();
    auto forwardQueue = rightP2PNetDev->GetQueue();
    returnQueue->SetMaxSize(QueueSize(returnQueueSize + "p"));
    forwardQueue->SetMaxSize(QueueSize(forwardQueueSize + "p"));

    // auto leftNode = sim.GetLeftNode();
    // auto leftDevice = leftNode->GetDevice(0);
    // leftDevice->SetDataRate(StringValue(returnDataRate))
    // leftDevice->SetQueue(StringValue(returnQueue))
    // auto leftPtr = devices.Get(0);
    // leftPtr->SetDataRate(StringValue(returnDataRate))
    // leftPtr->SetQueue(StringValue(returnQueue))
    //
    // auto rightPtr = devices.Get(1);
    // rightPtr->SetDataRate(StringValue(forwardDataRate))
    // rightPtr->SetQueue(StringValue(forwardQueue))

    sim.Run(Seconds(36000));
}
