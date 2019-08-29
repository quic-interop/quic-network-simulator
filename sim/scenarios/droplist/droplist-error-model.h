#ifndef DROPLIST_ERROR_MODEL_H
#define DROPLIST_ERROR_MODEL_H

#include "ns3/error-model.h"

using namespace ns3;

// The DroplistErrorModel drops packets enumerated by the user. This model does
// a simple packet count and drops the specified packets. Packet numbering
// starts at 1.

class DroplistErrorModel : public ErrorModel {
public:
  static TypeId GetTypeId(void);
  DroplistErrorModel();
  void SetDrops(std::string drops_in);

private:
  std::map<int, bool> drops;
  bool DoCorrupt (Ptr<Packet> p);
  void DoReset(void);
};

#endif /* DROPLIST_ERROR_MODEL_H */
