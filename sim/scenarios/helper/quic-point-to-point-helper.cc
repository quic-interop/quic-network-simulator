#include "ns3/core-module.h"
#include "ns3/traffic-control-helper.h"
#include "quic-point-to-point-helper.h"

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

  return devices;
}
