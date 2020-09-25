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
  em->DoRebind();
  if (!next.IsZero())
    Simulator::Schedule(next, &rebind, em, next);
}

int main(int argc, char *argv[]) {
  string delay, bandwidth, queue, first_rebind = "0s", rebind_freq = "0s";
  bool cgn = false;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
  cmd.AddValue("first_rebind", "time of first NAT rebind (e.g., 3s)",
               first_rebind);
  cmd.AddValue("rebind_freq",
               "NAT rebind frequency (e.g., 3s; 0 to disable repeated rebinds)",
               rebind_freq);
  cmd.AddValue("cgn", "act as CGN; change client IP address when rebinding",
               cgn);
  cmd.Parse(argc, argv);

  NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
  NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
  NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");

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
  em->SetCGN(cgn);
  em->Enable();

  devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  cout << Simulator::Now().GetSeconds() << "s: first NAT rebind in "
       << first_rebind << ", frequency " << rebind_freq << endl;

  if (!Time(first_rebind).IsZero())
    Simulator::Schedule(Time(first_rebind), &rebind, em, Time(rebind_freq));
  sim.Run(Seconds(36000));
}
