#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

const Time print_interval = Seconds(5);

void printBW(Ptr<PacketSink> sink) {
  static uint64_t last_rec = 0;
  static Time last_time;

  uint64_t current = sink->GetTotalRx();
  Time now = Simulator::Now();

  std::cout << "Bandwidth: " << 8 * (current - last_rec) / ((now - last_time).GetSeconds() * 1000) << " Kbps" << std::endl;
  last_time = now;
  last_rec = current;
  Simulator::Schedule(print_interval, &printBW, sink);
}

int main(int argc, char *argv[]) {
  std::string delay, bandwidth, queue;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
  cmd.Parse(argc, argv);

  QuicNetworkSimulatorHelper sim;

  QuicPointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  p2p.SetQueueSize(StringValue(queue + "p"));

  NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("192.168.50.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  // Create a UDP packet source on the left node.
  uint16_t port = 9;   // Discard port (RFC 863)
  
  BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces.GetAddress(1), port));
  source.SetAttribute("MaxBytes", UintegerValue(0)); // unlimited
  ApplicationContainer sourceApps = source.Install(sim.GetLeftNode());
  sourceApps.Start(Seconds(0));

  // Create a sink to receive the packets on the right node.
  PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer apps = sink.Install(sim.GetRightNode());
  apps.Start(Seconds(0));

  Simulator::Schedule(print_interval, &printBW, apps.Get(0)->GetObject<PacketSink>());

  sim.Run(Seconds(36000));
}
