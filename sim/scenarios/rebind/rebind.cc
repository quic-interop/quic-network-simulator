#include "ns3/string.h"

#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "rebind-error-model.h"

using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

void rebind(Ptr<RebindErrorModel> em, const Time next) {
  em->DoRebind();
  if (!next.IsZero())
    Simulator::Schedule(next, &rebind, em, next);
}

int main(int argc, char *argv[]) {
  string delay, bandwidth, queue, first_rebind = "0s", rebind_freq = "0s";
  bool rebind_addr = false;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
  cmd.AddValue("first-rebind", "time of first rebind (e.g., 3s)", first_rebind);
  cmd.AddValue("rebind-freq", "rebind frequency after first rebind",
               rebind_freq);
  cmd.AddValue("rebind-addr", "change client IP address when rebinding",
               rebind_addr);
  cmd.Parse(argc, argv);

  NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
  NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
  NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");
  NS_ABORT_MSG_IF(first_rebind.length() == 0,
                  "Missing parameter: first_rebind");
  NS_ABORT_MSG_IF(rebind_freq.length() > 0 && Time(first_rebind).IsZero(),
                  "Illegal parameter value: rebind_freq is zero");

  QuicNetworkSimulatorHelper sim;

  // Stick in the point-to-point line between the sides.
  QuicPointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  p2p.SetQueueSize(StringValue(queue + "p"));

  NetDeviceContainer devices =
      p2p.Install(sim.GetLeftNode(), sim.GetRightNode());

  Ptr<RebindErrorModel> em = CreateObject<RebindErrorModel>();
  em->SetRebindAddr(rebind_addr);
  em->Enable();

  devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));
  devices.Get(1)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  cout << Simulator::Now().GetSeconds() << "s: first rebind in "
       << first_rebind;
  if (rebind_freq.length() > 0)
    cout << ", frequency " << rebind_freq;
  cout << endl;

  Simulator::Schedule(Time(first_rebind), &rebind, em, Time(rebind_freq));
  sim.Run(Seconds(36000));
}
