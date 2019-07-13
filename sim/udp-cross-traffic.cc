#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "helper/quic-network-simulator-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {
  std::string delay, bandwidth, cross_data_rate;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("crossdatarate", "data rate of the cross traffic", cross_data_rate);
  cmd.Parse (argc, argv);

  QuicNetworkSimulatorHelper sim;

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));

  NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.50.0.0", "255.255.0.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  // Create a UDP packet source on the left node.
  uint16_t port = 9;   // Discard port (RFC 863)
  OnOffHelper onoff("ns3::UdpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
  onoff.SetConstantRate(DataRate(cross_data_rate));
  onoff.Install(sim.GetLeftNode()).Start(Seconds(20));

  p2p.EnablePcapAll("trace");

  // Create a sink to receive the packets on the right node.
  PacketSinkHelper sink("ns3::UdpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  sink.Install(sim.GetRightNode()).Start(Seconds(20));
 
  sim.Run(Seconds(36000));
}
