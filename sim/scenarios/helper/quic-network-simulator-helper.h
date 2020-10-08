#ifndef QUIC_NETWORK_SIMULATOR_HELPER_H
#define QUIC_NETWORK_SIMULATOR_HELPER_H

#include "ns3/node.h"

using namespace ns3;

class QuicNetworkSimulatorHelper {
public:
  QuicNetworkSimulatorHelper();
  void Run(Time);
  Ptr<Node> GetLeftNode() const;
  Ptr<Node> GetRightNode() const;

private:
  void RunSynchronizer() const;
  Ptr<Node> left_node_, right_node_;
};

#endif /* QUIC_NETWORK_SIMULATOR_HELPER_H */
