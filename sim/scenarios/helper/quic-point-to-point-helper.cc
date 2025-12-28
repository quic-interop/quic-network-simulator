#include "ns3/core-module.h"
#include "ns3/traffic-control-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/ipv6-address-helper.h"
#include "quic-point-to-point-helper.h"
#include "network-config.h"

using namespace ns3;

QuicPointToPointHelper::QuicPointToPointHelper() : queue_size_(StringValue("100p")) {
  SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
}

void QuicPointToPointHelper::SetQueueSize(StringValue size) {
  queue_size_ = size;
}

NetDeviceContainer QuicPointToPointHelper::Install(Ptr<Node> a, Ptr<Node> b) {
  NetDeviceContainer devices = PointToPointHelper::Install(a, b);
  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize", queue_size_);
  tch.Install(devices);

  const NetworkConfig& config = NetworkConfig::Instance();
  Ipv4AddressHelper ipv4;
  ipv4.SetBase(config.GetV4PointToPointNetwork().c_str(), config.GetV4SubnetMask().c_str());
  ipv4.Assign(devices);

  Ipv6AddressHelper ipv6;
  ipv6.SetBase(config.GetV6PointToPointNetwork().c_str(), config.GetV6PrefixInt());
  ipv6.Assign(devices);

  return devices;
}
