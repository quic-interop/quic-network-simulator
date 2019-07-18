#include "ns3/core-module.h"
#include "ns3/traffic-control-helper.h"
#include "quic-point-to-point-helper.h"

using namespace ns3;

NetDeviceContainer QuicPointToPointHelper::Install(Ptr<Node> a, Ptr<Node> b) {
  NetDeviceContainer devices = PointToPointHelper::Install(a, b);
  
  TrafficControlHelper tch;
  tch.SetRootQueueDisc("ns3::PfifoFastQueueDisc", "MaxSize", StringValue("1p"));
  tch.Install(devices);

  return devices;
}
