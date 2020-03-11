#ifndef DROPRATE_ERROR_MODEL_H
#define DROPRATE_ERROR_MODEL_H

#include <set>
#include <random>
#include "ns3/error-model.h"

using namespace ns3;

// The DropRateErrorModel drops random packets, at approximately a
// user-specified drop rate. However, it will not create burst losses of more
// than 3 packets.
class DropRateErrorModel : public ErrorModel {
 public:
    static TypeId GetTypeId(void);
    DropRateErrorModel();
    void SetDropRate(int perc);
    
 private:
    int rate;
    int consecutivelyCorrupted;
    std::mt19937 *rng;
    std::uniform_int_distribution<> distr;

    bool DoCorrupt (Ptr<Packet> p);
    void DoReset(void);
};

#endif /* DROPRATE_ERROR_MODEL_H */
