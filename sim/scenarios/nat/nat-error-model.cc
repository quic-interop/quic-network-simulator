#include "nat-error-model.h"
#include "ns3/core-module.h"
#include <cassert>

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(NATErrorModel);

TypeId NATErrorModel::GetTypeId(void) {
  static TypeId tid = TypeId("NATErrorModel")
                          .SetParent<ErrorModel>()
                          .AddConstructor<NATErrorModel>();
  return tid;
}

NATErrorModel::NATErrorModel()
    : client("193.167.0.100"), server("193.167.100.100"), nat(client),
      change_client_addr(false) {
  rng = CreateObject<UniformRandomVariable>();
}

void NATErrorModel::DoReset() {}

void NATErrorModel::SetAddrChangeFlag(bool change_addr) {
  change_client_addr = change_addr;
}

void NATErrorModel::DoRebind() {
  for (auto &b : fwd) {
    assert(rev.size() < UINT16_MAX - 1);
    const uint16_t old_port = b.second;
    do
      b.second = rng->GetInteger(1, UINT16_MAX);
    while (rev.find(b.second) != rev.end());
    rev[b.second] = b.first;
    rev[old_port] = 0;
    cout << "change: " << old_port << " is now " << b.second << endl;
  }

  for (auto &b : fwd)
    cout << "fwd: " << b.first << " -> " << b.second << endl;

  for (auto &b : rev)
    cout << "rev: " << b.first << " -> " << b.second << endl;

  // XXX change nat addr
}

bool NATErrorModel::DoCorrupt(Ptr<Packet> p) {
  QuicPacket qp = QuicPacket(p);

  const Ipv4Address &src_ip_in = qp.GetIpv4Header().GetSource();
  const Ipv4Address &dst_ip_in = qp.GetIpv4Header().GetDestination();
  const uint16_t src_port_in = qp.GetUdpHeader().GetSourcePort();
  const uint16_t dst_port_in = qp.GetUdpHeader().GetDestinationPort();

  cout << Simulator::Now().GetSeconds() << "s: IN from=" << src_ip_in << ":"
       << src_port_in << " to=" << dst_ip_in << ":" << dst_port_in << endl;

  if (src_ip_in == client) {
    cout << "from client" << endl;
    if (fwd.find(src_port_in) == fwd.end()) {
      fwd[src_port_in] = src_port_in;
      rev[src_port_in] = src_port_in;
    }
    qp.GetUdpHeader().SetSourcePort(fwd[src_port_in]);
    qp.GetIpv4Header().SetSource(client);
  } else if (src_ip_in == server) {
    cout << "from server" << endl;
    qp.GetUdpHeader().SetDestinationPort(rev[dst_port_in]);
    if (rev[dst_port_in] == 0) {
      cout << "DROPPING" << endl;
      return true;
    }
  } else {
    cout << "UNKNOWN src_ip_in " << src_ip_in << endl;
    return true;
  }

  qp.ReassemblePacket();

  const Ipv4Address &src_ip_out = qp.GetIpv4Header().GetSource();
  const Ipv4Address &dst_ip_out = qp.GetIpv4Header().GetDestination();
  const uint16_t src_port_out = qp.GetUdpHeader().GetSourcePort();
  const uint16_t dst_port_out = qp.GetUdpHeader().GetDestinationPort();

  cout << "OUT from=" << src_ip_out << ":" << src_port_out
       << " to=" << dst_ip_out << ":" << dst_port_out << endl;

  return false;
}
