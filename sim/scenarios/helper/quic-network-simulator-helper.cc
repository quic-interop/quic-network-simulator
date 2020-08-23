#include <csignal>
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

void onSignal(int signum) {
  std::cout << "Received signal: " << signum << std::endl;
  // see https://gitlab.com/nsnam/ns-3-dev/issues/102
  Simulator::Stop();
  NS_FATAL_ERROR(signum);
}

void installNetDevice(Ptr<Node> node, std::string deviceName, Mac48AddressValue macAddress, Ipv4InterfaceAddress ipv4Address, Ipv6InterfaceAddress ipv6Address) {
  EmuFdNetDeviceHelper emu;
  emu.SetDeviceName(deviceName);
  NetDeviceContainer devices = emu.Install(node);
  Ptr<NetDevice> device = devices.Get(0);
  device->SetAttribute("Address", macAddress);

  Ptr<Ipv4> ipv4 = node->GetObject<Ipv4>();
  uint32_t interface_v4 = ipv4->AddInterface(device);
  ipv4->AddAddress(interface_v4, ipv4Address);
  ipv4->SetMetric(interface_v4, 1);
  ipv4->SetUp(interface_v4);

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
  uint32_t interface_v6 = ipv6->AddInterface(device);
  ipv6->AddAddress(interface_v6, ipv6Address);
  ipv6->SetMetric(interface_v6, 1);
  ipv6->SetUp(interface_v6);
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

  installNetDevice(left_node_, "eth0", Mac48AddressValue("02:51:55:49:43:00"), Ipv4InterfaceAddress("193.167.0.2", "255.255.255.0"), Ipv6InterfaceAddress("fd00::2", "ffff:ffff:ffff:ffff:ffff:ffff:ffff::"));
  installNetDevice(right_node_, "eth1", Mac48AddressValue("02:51:55:49:43:01"), Ipv4InterfaceAddress("193.167.100.2", "255.255.255.0"), Ipv6InterfaceAddress("fd00::100:2", "ffff:ffff:ffff:ffff:ffff:ffff:ffff::"));
}

void QuicNetworkSimulatorHelper::Run(Time duration) {
  signal(SIGTERM, onSignal);
  signal(SIGINT, onSignal);
  signal(SIGKILL, onSignal);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();
  // write the IPv4 routing table to file
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes.ipv4", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);

  // write the IPv6 routing table to file
  Ptr<OutputStreamWrapper> routingStream_v6 = Create<OutputStreamWrapper>("dynamic-global-routing.routes.ipv6", std::ios::out);
  Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream_v6);

  Simulator::Stop(duration);
  RunSynchronizer();
  Simulator::Run();
  Simulator::Destroy();
}

void QuicNetworkSimulatorHelper::RunSynchronizer() const {
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  NS_ABORT_MSG_IF(sockfd < 0, "ERROR opening socket");

  struct sockaddr_in addr;
  bzero((char *) &addr, sizeof(addr));

  addr.sin_family = AF_INET;  
  addr.sin_addr.s_addr = INADDR_ANY;  
  addr.sin_port = htons(57832);

  int res = bind(sockfd, (struct sockaddr *) &addr, sizeof(addr));
  NS_ABORT_MSG_IF(res < 0, "ERROR on binding");
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
