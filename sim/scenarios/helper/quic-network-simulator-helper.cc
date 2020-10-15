#include <cassert>
#include <csignal>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>

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
  uint32_t interface = ipv4->AddInterface(device);
  ipv4->AddAddress(interface, ipv4Address);
  ipv4->SetMetric(interface, 1);
  ipv4->SetUp(interface);

  Ptr<Ipv6> ipv6 = node->GetObject<Ipv6>();
  ipv6->SetAttribute("IpForward", BooleanValue(true));
  interface = ipv6->AddInterface(device);
  ipv6->AddAddress(interface, ipv6Address);
  ipv6->SetMetric(interface, 1);
  ipv6->SetUp(interface);
}

Mac48Address getMacAddress(std::string iface) {
  unsigned char buf[6];
  int fd = socket(AF_INET, SOCK_DGRAM, 0);
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ-1);
  ioctl(fd, SIOCGIFHWADDR, &ifr);
  for(unsigned int i=0; i<6; i++) {
    buf[i] = ifr.ifr_hwaddr.sa_data[i];
  }
  ioctl(fd, SIOCGIFMTU, &ifr);
  close(fd);
  Mac48Address mac;
  mac.CopyFrom(buf);
  return mac;
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

  installNetDevice(left_node_, "eth0", getMacAddress("eth0"), Ipv4InterfaceAddress("193.167.0.2", "255.255.255.0"), Ipv6InterfaceAddress("fd00:cafe:cafe:0::2", 64));
  installNetDevice(right_node_, "eth1", getMacAddress("eth1"), Ipv4InterfaceAddress("193.167.100.2", "255.255.255.0"), Ipv6InterfaceAddress("fd00:cafe:cafe:100::2", 64));
}

void massageIpv6Routing(Ptr<Node> local, Ptr<Node> peer) {
  Ptr<Ipv6StaticRouting> routing = Ipv6RoutingHelper::GetRouting<Ipv6StaticRouting>(local->GetObject<Ipv6>()->GetRoutingProtocol());
  Ptr<Ipv6> peer_ipv6 = peer->GetObject<Ipv6>();
  Ipv6Address dst;
  for (uint32_t i = 0; i < peer_ipv6->GetNInterfaces(); i++)
    for (uint32_t j = 0; j < peer_ipv6->GetNAddresses(i); j++)
      if (peer_ipv6->GetAddress(i, j).GetAddress().CombinePrefix(64) == "fd00:cafe:cafe:50::") {
        dst = peer_ipv6->GetAddress(i, j).GetAddress();
        goto done;
      }

done:
  assert(dst.IsInitialized());
  routing->SetDefaultRoute(dst, 2);
}

void QuicNetworkSimulatorHelper::Run(Time duration) {
  signal(SIGTERM, onSignal);
  signal(SIGINT, onSignal);
  signal(SIGKILL, onSignal);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  // Ipv6GlobalRoutingHelper does not exist - fake it
  massageIpv6Routing(left_node_, right_node_);
  massageIpv6Routing(right_node_, left_node_);

  // write the routing table to file
  Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper>("dynamic-global-routing.routes", std::ios::out);
  Ipv4RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);
  Ipv6RoutingHelper::PrintRoutingTableAllAt(Seconds(0.), routingStream);

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
