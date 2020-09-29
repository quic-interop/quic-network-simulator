#include "rebind-error-model.h"
#include "ns3/core-module.h"
#include <cassert>

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(RebindErrorModel);

TypeId RebindErrorModel::GetTypeId(void) {
  static TypeId tid = TypeId("RebindErrorModel")
                          .SetParent<ErrorModel>()
                          .AddConstructor<RebindErrorModel>();
  return tid;
}

RebindErrorModel::RebindErrorModel()
    : client("193.167.0.100"), server("193.167.100.100"), nat(client),
      rebind_addr(false) {
  rng = CreateObject<UniformRandomVariable>();
}

void RebindErrorModel::DoReset() {}

void RebindErrorModel::SetRebindAddr(bool ra) { rebind_addr = ra; }

void RebindErrorModel::DoRebind() {
  const Ipv4Address old_nat = nat;
  if (rebind_addr)
    do {
      nat.Set((old_nat.Get() & 0xffffff00) | rng->GetInteger(1, 0xfe));
    } while (nat == old_nat || nat == client);

  for (auto &b : fwd) {
    // FIXME: this will abort if we run out of ports (= after 64K rebinds)
    assert(rev.size() < UINT16_MAX - 1);
    const uint16_t old_port = b.second;
    do
      b.second = rng->GetInteger(1, UINT16_MAX);
    while (rev.find(b.second) != rev.end());
    rev[b.second] = b.first;
    rev[old_port] = 0;
    cout << Simulator::Now().GetSeconds() << "s: "
         << " rebinding: " << old_nat << ":" << old_port << " -> " << nat << ":"
         << b.second << endl;
  }
}

bool RebindErrorModel::DoCorrupt(Ptr<Packet> p) {
  QuicPacket qp = QuicPacket(p);

  const Ipv4Address &src_ip_in = qp.GetIpv4Header().GetSource();
  const Ipv4Address &dst_ip_in = qp.GetIpv4Header().GetDestination();
  const uint16_t src_port_in = qp.GetUdpHeader().GetSourcePort();
  const uint16_t dst_port_in = qp.GetUdpHeader().GetDestinationPort();

  if (src_ip_in == client) {
    if (fwd.find(src_port_in) == fwd.end())
      rev[src_port_in] = fwd[src_port_in] = src_port_in;
    qp.GetUdpHeader().SetSourcePort(fwd[src_port_in]);
    qp.GetIpv4Header().SetSource(nat);

  } else if (src_ip_in == server) {
    if (rev[dst_port_in] == 0) {
      cout << "unknown binding for destination " << dst_ip_in << ":"
           << dst_port_in << ", dropping packet" << endl;
      return true;
    }
    qp.GetIpv4Header().SetDestination(client);
    qp.GetUdpHeader().SetDestinationPort(rev[dst_port_in]);

  } else {
    cout << "unknown source " << src_ip_in << ", dropping packet" << endl;
    return true;
  }

  qp.ReassemblePacket();
  return false;
}
