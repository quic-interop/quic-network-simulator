#include "nat-error-model.h"
#include "ns3/ipv4-queue-disc-item.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(NATErrorModel);

TypeId NATErrorModel::GetTypeId(void) {
  static TypeId tid = TypeId("NATErrorModel")
                          .SetParent<ErrorModel>()
                          .AddConstructor<NATErrorModel>();
  return tid;
}

NATErrorModel::NATErrorModel() { rng = CreateObject<UniformRandomVariable>(); }

void NATErrorModel::DoReset(void) { bindings.clear(); }

bool NATErrorModel::DoCorrupt(Ptr<Packet> p) {
  QuicPacket qp = QuicPacket(p);

  const std::pair<uint64_t, uint64_t> tuple(
      (uint64_t)qp.GetIpv4Header().GetSource().Get() << 32 |
          qp.GetUdpHeader().GetSourcePort(),
      (uint64_t)qp.GetIpv4Header().GetDestination().Get() << 32 |
          qp.GetUdpHeader().GetDestinationPort());

  auto b = bindings.find(tuple);
  if (b == bindings.end())
    // new flow
    bindings.insert({tuple, qp.GetUdpHeader().GetSourcePort()});
  else
    // use source port from binding table
    qp.GetUdpHeader().SetSourcePort(b->second);

  qp.ReassemblePacket();
  return false;
}
