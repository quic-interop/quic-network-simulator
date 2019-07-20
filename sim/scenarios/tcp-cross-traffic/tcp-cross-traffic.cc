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

  NodeContainer nodes;
  nodes.Create(2);
  InternetStackHelper internet;
  internet.Install(nodes);
  Ptr<Node> source_node = nodes.Get(1);
  Ptr<Node> sink_node = nodes.Get(0);

  QuicPointToPointHelper p2p_source;
  p2p_source.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p_source.SetChannelAttribute("Delay", StringValue("1ms"));

  NetDeviceContainer devices_source = p2p_source.Install(sim.GetRightNode(), source_node);
  Ipv4AddressHelper ipv4_source;
  ipv4_source.SetBase("192.168.49.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_source = ipv4_source.Assign(devices_source);

  QuicPointToPointHelper p2p_sink;
  p2p_sink.SetDeviceAttribute("DataRate", StringValue("100Mbps"));
  p2p_sink.SetChannelAttribute("Delay", StringValue("1ms"));

  NetDeviceContainer devices_sink = p2p_sink.Install(sink_node, sim.GetLeftNode());
  Ipv4AddressHelper ipv4_sink;
  ipv4_sink.SetBase("192.168.51.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces_sink = ipv4_sink.Assign(devices_sink);

  uint16_t port = 9;   // Discard port (RFC 863)
  Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1448));
  BulkSendHelper source("ns3::TcpSocketFactory", InetSocketAddress(interfaces_sink.GetAddress(0), port));
  source.SetAttribute("MaxBytes", UintegerValue(0)); // unlimited
  ApplicationContainer source_apps = source.Install(source_node);
  source_apps.Start(Seconds(0));

  PacketSinkHelper sink("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
  ApplicationContainer apps = sink.Install(sink_node);
  apps.Start(Seconds(0));

  p2p_source.EnablePcapAll("trace-source");
  p2p.EnablePcapAll("trace");
  p2p_sink.EnablePcapAll("trace-sink");

  Simulator::Schedule(print_interval, &printBW, apps.Get(0)->GetObject<PacketSink>());

  sim.Run(Seconds(36000));
}
