#ifndef DROPLIST_ERROR_MODEL_H
#define DROPLIST_ERROR_MODEL_H

#include <set>
#include "ns3/error-model.h"

using namespace ns3;

// The DroplistErrorModel drops packets enumerated by the user. This model does
// a simple packet count and drops the specified packets. Packet numbering
// starts at 1.

class DroplistErrorModel : public ErrorModel {
 public:
    static TypeId GetTypeId(void);
    DroplistErrorModel();
    void SetDrop(int packet_num);
    
 private:
    std::set<int> drops;
    bool DoCorrupt (Ptr<Packet> p);
    void DoReset(void);
};

#endif /* DROPLIST_ERROR_MODEL_H */
