#ifndef QUIC_POINT_TO_POINT_HELPER_H
#define QUIC_POINT_TO_POINT_HELPER_H

#include "ns3/point-to-point-module.h"

using namespace ns3;

// The QuicPointToPointHelper acts like the ns3::PointToPointHelper,
// but it sets the PfifoFastQueueDisc queue size to 1, preventing the build-up
// of traffic control layer queues and minimizing the impact on the latency.
// Note that you can still set a network queue using PointToPointHelper::SetQueue.
class QuicPointToPointHelper : public PointToPointHelper {
public:
  NetDeviceContainer Install(Ptr<Node> a, Ptr<Node> b);
};

#endif /* QUIC_POINT_TO_POINT_HELPER_H */
