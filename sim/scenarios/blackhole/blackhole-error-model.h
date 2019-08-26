#ifndef BLACKHOLE_ERROR_MODEL_H
#define BLACKHOLE_ERROR_MODEL_H

#include "ns3/error-model.h"

using namespace ns3;

// The BlackholeErrorModel drops all packets.
class BlackholeErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId(void);
  BlackholeErrorModel();

  void Enable();
  void Disable();
 
private:
  bool enabled_;
  bool DoCorrupt (Ptr<Packet> p);
  void DoReset(void);
};

#endif /* BLACKHOLE_ERROR_MODEL_H */
