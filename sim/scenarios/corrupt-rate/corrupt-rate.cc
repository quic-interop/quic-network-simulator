#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "corrupt-rate-error-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {
    std::string delay, bandwidth, queue, client_rate, server_rate,
        client_burst, server_burst;
    std::random_device rand_dev;
    std::mt19937 generator(rand_dev());  // Seed random number generator first
    Ptr<CorruptRateErrorModel> client_corrupts = CreateObject<CorruptRateErrorModel>();
    Ptr<CorruptRateErrorModel> server_corrupts = CreateObject<CorruptRateErrorModel>();
    CommandLine cmd;
    
    cmd.AddValue("delay", "delay of the p2p link", delay);
    cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
    cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
    cmd.AddValue("rate_to_client", "packet corruption rate (towards client)", client_rate);
    cmd.AddValue("rate_to_server", "packet corruption rate (towards server)", server_rate);
    cmd.AddValue("burst_to_client",
                 "max. packet corruption burst length (towards client)",
                 client_burst);
    cmd.AddValue("burst_to_server",
                 "max. packet corruption burst length (towards server)",
                 server_burst);
    cmd.Parse (argc, argv);
    
    NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
    NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");
    NS_ABORT_MSG_IF(client_rate.length() == 0, "Missing parameter: rate_to_client");
    NS_ABORT_MSG_IF(server_rate.length() == 0, "Missing parameter: rate_to_server");

    // Set client and server corruption rates.
    client_corrupts->SetCorruptRate(stoi(client_rate));
    if (client_burst.length() > 0)
        client_corrupts->SetMaxCorruptBurst(stoi(client_burst));
    server_Corrupts->SetCorruptRate(stoi(server_rate));
    if (server_burst.length() > 0)
        server_corrupts->SetMaxDropBurst(stoi(server_burst));

    QuicNetworkSimulatorHelper sim;

    // Stick in the point-to-point line between the sides.
    QuicPointToPointHelper p2p;
    p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
    p2p.SetChannelAttribute("Delay", StringValue(delay));
    p2p.SetQueueSize(StringValue(queue + "p"));
    
    NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
    
    devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(client_corrupts));
    devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(server_corrupts));
    
    sim.Run(Seconds(36000));
}
