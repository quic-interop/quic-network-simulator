#ifndef CORRUPTRATE_ERROR_MODEL_H
#define CORRUPTRATE_ERROR_MODEL_H

#include <set>
#include <random>
#include "ns3/error-model.h"

using namespace ns3;

// The CorruptRateErrorModel corrupts random packets, at a user-specified rate.
// But no more than a user-specified number of packets in a row.
class CorruptRateErrorModel : public RateErrorModel {
 public:
    static TypeId GetTypeId(void);
    CorruptRateErrorModel();
    void SetCorruptRate(int perc);
    void SetMaxCorruptBurst(int burst);
    
 private:
    int rate;
    std::mt19937 *rng;
    std::uniform_int_distribution<> distr;
    int burst;
    int corrupted_in_a_row;
    int corrupted;
    int forwarded;

    bool DoCorrupt (Ptr<Packet> p);
    void DoReset(void);
};

#endif /* CORRUPTRATE_ERROR_MODEL_H */
