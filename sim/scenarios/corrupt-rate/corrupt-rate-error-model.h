#ifndef CORRUPTRATE_ERROR_MODEL_H
#define CORRUPTRATE_ERROR_MODEL_H

#include <set>
#include <random>
#include "ns3/error-model.h"

using namespace ns3;

// The CorruptRateErrorModel corrupts random packets, at a user-specified rate.
class CorruptRateErrorModel : public RateErrorModel {
 public:
    static TypeId GetTypeId(void);
    CorruptRateErrorModel();
    void SetCorruptRate(int perc);
    
 private:
    int rate;
    std::mt19937 *rng;
    std::uniform_int_distribution<> distr;

    bool DoCorrupt (Ptr<Packet> p);
    void DoReset(void);
};

#endif /* CORRUPTRATE_ERROR_MODEL_H */
