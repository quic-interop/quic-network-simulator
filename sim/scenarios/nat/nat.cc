#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "nat-error-model.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

void rebind(Ptr<NATErrorModel> em, const Time next) {
  std::cout << Simulator::Now().GetSeconds() << "s: NAT rebinding" << std::endl;
  Simulator::Schedule(next, &rebind, em, next);

  // FIXME: This just renumbers source ports, without considering IP addresses.
  // So it can't support more than 64K flows.

  // FIXME: There is no removal of stale binding entries, so this code will not
  // support long-running simulations with many flows.

  std::set<uint16_t> used;
  for (auto &b : em->bindings) {
    const auto old = b.second;
    do {
      b.second = em->rng->GetInteger(1, UINT16_MAX);
    } while (b.second == old || used.find(b.second) != used.end());
    used.insert(b.second);
  }
}

int main(int argc, char *argv[]) {
  std::string delay, bandwidth, queue, rebind_freq;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
  cmd.AddValue("rebind", "NAT rebind frequency (e.g., 3s)", rebind_freq);
  cmd.Parse(argc, argv);

  NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
  NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
  NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");
  NS_ABORT_MSG_IF(rebind_freq.length() == 0, "Missing parameter: rebind");

  QuicNetworkSimulatorHelper sim;

  // Stick in the point-to-point line between the sides.
  QuicPointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  p2p.SetQueueSize(StringValue(queue + "p"));

  NetDeviceContainer devices =
      p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("193.167.50.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  Ptr<NATErrorModel> em = CreateObject<NATErrorModel>();
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  Simulator::Schedule(Time(rebind_freq), &rebind, em, Time(rebind_freq));
  sim.Run(Seconds(36000));
}
