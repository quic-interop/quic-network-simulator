#include <cstdint>
#include <vector>

#include "corrupt-rate-error-model.h"
#include "../helper/quic-packet.h"
#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/ppp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(CorruptRateErrorModel);

TypeId CorruptRateErrorModel::GetTypeId(void) {
    static TypeId tid = TypeId("CorruptRateErrorModel")
        .SetParent<ErrorModel>()
        .AddConstructor<CorruptRateErrorModel>()
        ;
    return tid;
}
 
CorruptRateErrorModel::CorruptRateErrorModel()
    : rate(0), distr(0, 99) {
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void CorruptRateErrorModel::DoReset(void) { }

bool CorruptRateErrorModel::DoCorrupt(Ptr<Packet> p) {
    if (distr(*rng) >= rate) return false;

    QuicPacket qp = QuicPacket(p);
    // Corrupt a byte in the 50 bytes of the UDP payload.
    // This way, we will frequenetly hit the QUIC header.
    std::uniform_int_distribution<> d(0,  min(uint32_t(50), p->GetSize() - 1));
    auto pos = d(*rng);
    vector<uint8_t>& payload = qp.GetUdpPayload();
    cout << "Corrupting byte " << pos << " of the UDP payload (total: " << payload.size() << ")" << endl;
    payload[pos] ^= 0xff;
    qp.ReassemblePacket();
    return false;
}

void CorruptRateErrorModel::SetCorruptRate(int rate_in) {
    rate = rate_in;
}
