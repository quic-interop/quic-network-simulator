/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */

#include "ns3/abort.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-apps-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/ipv4-list-routing-helper.h"
#include "ns3/point-to-point-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

void installNetDevice(Ptr<Node> node, std::string deviceName, Mac48AddressValue macAddress, Ipv4InterfaceAddress ipv4Address) {
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName(deviceName);
  NetDeviceContainer devices = emu.Install(node);
  Ptr<NetDevice> device = devices.Get(0);
  device->SetAttribute("Address", macAddress);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface = ipv4->AddInterface(device);
  ipv4->AddAddress(interface, ipv4Address);
  ipv4->SetMetric(interface, 1);
  ipv4->SetUp(interface);
}


int main(int argc, char *argv[]) {
  GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NodeContainer nodesLeft;
  nodesLeft.Create(1);
  InternetStackHelper internetLeft;
  internetLeft.Install(nodesLeft);

  NodeContainer nodesRight;
  nodesRight.Create(1);
  InternetStackHelper internetRight;
  internetRight.Install(nodesRight);

  // Stick in the point-to-point line between the sides.
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue("10Mbps"));
  p2p.SetChannelAttribute("Delay", StringValue("10ms"));

  NodeContainer nodes = NodeContainer(nodesLeft.Get(0), nodesRight.Get(0));
  NetDeviceContainer devices = p2p.Install(nodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("10.50.0.0", "255.255.0.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  NS_LOG_INFO("Create eth0");
  installNetDevice(nodesLeft.Get(0), "eth0", Mac48AddressValue("02:51:55:49:43:00"), Ipv4InterfaceAddress("10.0.0.2", "255.255.0.0"));
  NS_LOG_INFO("Create eth1");
  installNetDevice(nodesRight.Get(0), "eth1", Mac48AddressValue("02:51:55:49:43:01"), Ipv4InterfaceAddress("10.100.0.2", "255.255.0.0"));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  // write the routing table to file
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);

  NS_LOG_INFO("Run Emulation.");
  Simulator::Stop(Seconds(36000.0));
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
}
