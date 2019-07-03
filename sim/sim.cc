/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "quic-network-simulator-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

int main(int argc, char *argv[]) {
  QuicNetworkSimulatorHelper sim;

  // Stick in the point-to-point line between the sides.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));

  NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.50.0.0", "255.255.0.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // write the routing table to file
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);

  NS_LOG_INFO("Run Emulation.");
  Simulator::Stop(Seconds(36000.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
}
