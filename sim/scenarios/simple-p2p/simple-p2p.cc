#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "../helper/quic-network-simulator-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {
  std::string delay, bandwidth, queue;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link", queue);
  cmd.Parse (argc, argv);

  QuicNetworkSimulatorHelper sim;

  // Stick in the point-to-point line between the sides.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  p2p.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(queue + "p"));

  NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("192.168.50.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  sim.Run(Seconds(36000));
}
