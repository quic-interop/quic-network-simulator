#ifndef DROPRATE_ERROR_MODEL_H
#define DROPRATE_ERROR_MODEL_H

#include <set>
#include <random>
#include "ns3/error-model.h"

using namespace ns3;

// The DropRateErrorModel drops random packets, at a user-specified drop rate. 
class DropRateErrorModel : public ErrorModel {
 public:
    static TypeId GetTypeId(void);
    DropRateErrorModel();
    void SetDropRate(int perc);

 private:
    int rate;
    std::mt19937 *rng;
    std::uniform_int_distribution<> distr;

    unsigned int drop_counter;
    unsigned int pass_counter;
    bool first_packet_logged;
    static constexpr unsigned int LOG_AFTER_PACKETS = 1000;

    bool DoCorrupt (Ptr<Packet> p);
    void DoReset(void);

    void log(void);
};

#endif /* DROPRATE_ERROR_MODEL_H */
