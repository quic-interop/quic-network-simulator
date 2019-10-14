#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

#include "ns3/core-module.h"
#include "ns3/fd-net-device-module.h"
#include "ns3/internet-module.h"
#include "quic-network-simulator-helper.h"

using namespace ns3;

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

QuicNetworkSimulatorHelper::QuicNetworkSimulatorHelper() {
  GlobalValue::Bind("SimulatorImplementationType", StringValue("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind("ChecksumEnabled", BooleanValue(true));

  NodeContainer nodes;
  nodes.Create(2);
  InternetStackHelper internet;
  internet.Install(nodes);

  left_node_ = nodes.Get(0);
  right_node_ = nodes.Get(1);

  NS_LOG_INFO("Create eth0");
  installNetDevice(left_node_, "eth0", Mac48AddressValue("02:51:55:49:43:00"), Ipv4InterfaceAddress("193.167.0.2", "255.255.255.0"));
  NS_LOG_INFO("Create eth1");
  installNetDevice(right_node_, "eth1", Mac48AddressValue("02:51:55:49:43:01"), Ipv4InterfaceAddress("193.167.100.2", "255.255.255.0"));
}

void QuicNetworkSimulatorHelper::Run(Time duration) {
  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  // write the routing table to file
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);

  NS_LOG_INFO("Run Emulation.");
  Simulator::Stop(duration);
  RunSynchronizer();
  Simulator::Run();
  Simulator::Destroy();
  NS_LOG_INFO("Done.");
}

void QuicNetworkSimulatorHelper::RunSynchronizer() const {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd < 0) NS_LOG_ERROR("ERROR opening socket");

  struct sockaddr_in addr;
  bzero((char *) &addr, sizeof(addr));

  addr.sin_family = AF_INET;  
  addr.sin_addr.s_addr = INADDR_ANY;  
  addr.sin_port = htons(57832);

  if(bind(sockfd, (struct sockaddr *) &addr, sizeof(addr)) < 0) NS_LOG_ERROR("ERROR on binding");
  // We never intend to accept any of the connections.
  // Use a large backlog queue instead.
  listen(sockfd, 100);
}

Ptr<Node> QuicNetworkSimulatorHelper::GetLeftNode() const {
  return left_node_;
}

Ptr<Node> QuicNetworkSimulatorHelper::GetRightNode() const {
  return right_node_;
}
