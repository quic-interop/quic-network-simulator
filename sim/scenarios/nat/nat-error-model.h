#ifndef NAT_ERROR_MODEL_H
#define NAT_ERROR_MODEL_H

#include "../helper/quic-packet.h"
#include "ns3/error-model.h"
#include "ns3/random-variable-stream.h"
#include <unordered_map>

using namespace ns3;

// The NATErrorModel drops packets enumerated by the user. This model does
// a simple packet count and drops the specified packets. Packet numbering
// starts at 1.

// A hash function used to hash a pair of any kind
struct hash_pair {
  template <class T1, class T2> size_t operator()(const pair<T1, T2> &p) const {
    auto hash1 = hash<T1>{}(p.first);
    auto hash2 = hash<T2>{}(p.second);
    return hash1 ^ hash2;
  }
};

class NATErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId(void);
  NATErrorModel();
  void SetDrop(int packet_num);
  Ptr<UniformRandomVariable> rng;
  void DoRebind();
  void SetCGN(bool cgn);

private:
  bool DoCorrupt(Ptr<Packet> p);
  void DoReset(void);
  Ipv4Address client, server, nat;
  bool do_cgn;

  unordered_map<uint16_t, uint16_t> fwd, rev;
};

#endif /* NAT_ERROR_MODEL_H */
