#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "droplist-error-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

void SetDrops(Ptr<DroplistErrorModel> drop_model, string drops_in) {
    char *cstr = new char[drops_in.length()+1];
    strcpy(cstr, drops_in.c_str());
    char *p = strtok(cstr,",");
    while (p) {
        drop_model->SetDrop(stoi(p));
        p = strtok(NULL,",");
    }
    delete[] cstr;
}

int main(int argc, char *argv[]) {
    std::string delay, bandwidth, queue, client_drops_in, server_drops_in;
    Ptr<DroplistErrorModel> client_drops = CreateObject<DroplistErrorModel>();
    Ptr<DroplistErrorModel> server_drops = CreateObject<DroplistErrorModel>();
    CommandLine cmd;
    
    cmd.AddValue("delay", "delay of the p2p link", delay);
    cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
    cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
    cmd.AddValue("drops_to_client", "list of packets (towards client) to drop", client_drops_in);
    cmd.AddValue("drops_to_server", "list of packets (towards server) to drop", server_drops_in);
    cmd.Parse (argc, argv);
    
    NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
    NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
    NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");

    // Set client and server droplists.
    SetDrops(client_drops, client_drops_in);
    SetDrops(server_drops, server_drops_in);

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
