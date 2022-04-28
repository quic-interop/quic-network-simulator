#ifndef GILBERT_ELLIOTT_DROP_MODEL_H
#define GILBERT_ELLIOTT_DROP_MODEL_H

#include <set>
#include <random>
#include "ns3/error-model.h"

using namespace ns3;

enum GilbertElliottState {
  GOOD,
  BAD,
};

// The model drops packets in the error state and passes packets in the normal state.
// The transition between both states is described by the transition probabilities.
class GilbertElliottDropModel : public ErrorModel {
 public:
    static TypeId GetTypeId(void);
    GilbertElliottDropModel();
    void SetProbabilities(float goodBadPerc, float badGoodPerc);

 private:
    float goodBadPerc;
    float badGoodPerc;
    GilbertElliottState state;

    std::mt19937 *rng;
    std::uniform_real_distribution<> distr;

    unsigned int drop_counter;
    unsigned int pass_counter;
    bool first_packet_logged;
    static constexpr unsigned int LOG_AFTER_PACKETS = 1000;

    bool DoCorrupt(Ptr<Packet> p);
    void DoReset(void);

    void log(void);
};

#endif /* GILBERT_ELLIOTT_DROP_MODEL_H */
