#include "ns3/core-module.h"
#include "ns3/error-model.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "../helper/quic-network-simulator-helper.h"
#include "../helper/quic-point-to-point-helper.h"
#include "blackhole-error-model.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("ns3 simulator");

void enable(Ptr<BlackholeErrorModel> em, const Time next, const int repeat) {
  static int counter = 0;
  counter++;
  std::cout << Simulator::Now().GetSeconds() << "s: Enabling blackhole" << std::endl;
  em->Enable();
  if(repeat > 0 && counter < repeat) {
    Simulator::Schedule(next, &enable, em, next, repeat);
  }
}

void disable(Ptr<BlackholeErrorModel> em, const Time next, const int repeat) {
  static int counter = 0;
  std::cout << Simulator::Now().GetSeconds() << "s: Disabling blackhole" << std::endl;
  counter++;
  em->Disable();
  if(repeat > 0 && counter < repeat) {
    Simulator::Schedule(next, &disable, em, next, repeat);
  }
}

int main(int argc, char *argv[]) {
  std::string delay, bandwidth, queue, on, off, repeat_s;
  CommandLine cmd;
  cmd.AddValue("delay", "delay of the p2p link", delay);
  cmd.AddValue("bandwidth", "bandwidth of the p2p link", bandwidth);
  cmd.AddValue("queue", "queue size of the p2p link (in packets)", queue);
  cmd.AddValue("on", "time that the connection is active (e.g. 15s)", on);
  cmd.AddValue("off", "time that the connection is dropping all packets (e.g. 2s)", off);
  cmd.AddValue("repeat", "(optional) turn the connection on and off this many times. Default: 1", repeat_s);
  cmd.Parse (argc, argv);

  NS_ABORT_MSG_IF(delay.length() == 0, "Missing parameter: delay");
  NS_ABORT_MSG_IF(bandwidth.length() == 0, "Missing parameter: bandwidth");
  NS_ABORT_MSG_IF(queue.length() == 0, "Missing parameter: queue");
  NS_ABORT_MSG_IF(on.length() == 0, "Missing parameter: on");
  NS_ABORT_MSG_IF(off.length() == 0, "Missing parameter: off");

  int repeat = 1;
  if(repeat_s.length() > 0) {
    repeat = std::stoi(repeat_s);
  }

  QuicNetworkSimulatorHelper sim;

  // Stick in the point-to-point line between the sides.
  QuicPointToPointHelper p2p;
  p2p.SetDeviceAttribute("DataRate", StringValue(bandwidth));
  p2p.SetChannelAttribute("Delay", StringValue(delay));
  p2p.SetQueueSize(StringValue(queue + "p"));

  NetDeviceContainer devices = p2p.Install(sim.GetLeftNode(), sim.GetRightNode());
  Ipv4AddressHelper ipv4;
  ipv4.SetBase("192.168.50.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign(devices);

  Ptr<BlackholeErrorModel> em = CreateObject<BlackholeErrorModel>();
  em->Disable();
  devices.Get(0)->SetAttribute("ReceiveErrorModel", PointerValue(em));

  Time intv = Time(on) + Time(off);
  Simulator::Schedule(Time(on), &enable, em, intv, repeat);
  Simulator::Schedule(intv, &disable, em, intv, repeat);

  sim.Run(Seconds(36000));
}
