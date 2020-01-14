#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "drop-rate-error-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {
    std::string delay, bandwidth, queue, client_rate, server_rate;
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());  // Seed random number generator first
    Ptr<DropRateErrorModel> client_drops = CreateObject<DropRateErrorModel>();
    Ptr<DropRateErrorModel> server_drops = CreateObject<DropRateErrorModel>();
    CommandLine cmd;
    
    cmd.AddValue("delay", "delay of the p2p link", delay);
    cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
    cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
    cmd.AddValue("rate_to_client", "packet drop rate (towards client)", client_rate);
    cmd.AddValue("rate_to_server", "packet drop rate (towards server)", server_rate);
    cmd.Parse (argc, argv);
    
    NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
    NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");
    NS_ABORT_MSG_IF(client_rate.length() == 0, "Missing parameter: rate_to_client");
    NS_ABORT_MSG_IF(server_rate.length() == 0, "Missing parameter: rate_to_server");

    // Set client and server drop rates.
    client_drops->SetDropRate(stoi(client_rate));
    server_drops->SetDropRate(stoi(server_rate));

    QuicNetworkSimulatorHelper sim;

    // Stick in the point-to-point line between the sides.
    QuicPointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    p2p.SetQueueSize(StringValue(queue + "p"));
    
    NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
    Ipv4AddressHelper ipv4;
    ipv4.SetBase("193.167.50.0", "255.255.255.0");
    Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);
    
    devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(client_drops));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(server_drops));
    
    sim.Run(Seconds(36000));
}
