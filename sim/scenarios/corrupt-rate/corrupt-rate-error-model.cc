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
    QuicPacket qp = QuicPacket(p);

    if(distr(*rng) >= rate) {
        cout << "Forwarding packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
        qp.ReassemblePacket();
        return false;
    }

    // Don't corrupt Version Negotiation packets.
    // Version Negotiation packets are expected to be sent when the version field of the Initial was corrupted.
    // Client are supposed to ignore Version Negotiation packets that contain the version that they offered.
    if(qp.IsVersionNegotiationPacket()) {
        qp.ReassemblePacket();
        return false;
    }
    // Corrupt a byte in the 50 bytes of the UDP payload.
    // This way, we will frequenetly hit the QUIC header.
    std::uniform_int_distribution<> d(0, min(uint32_t(50), p->GetSize() - 1));
    int pos = d(*rng);
    vector<uint8_t>& payload = qp.GetUdpPayload();
    // Replace the byte at position pos with a random value.
    while(true) {
        uint8_t n = std::uniform_int_distribution<>(0, 255)(*rng);
        if(payload[pos] == n) continue;
        cout << "Corrupted packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << " at offset " << pos << " (0x" << std::hex << (unsigned int) payload[pos] << " -> 0x" << (unsigned int) n << ")" << std::dec << endl;
        payload[pos] = n;
        break;
    }
    qp.ReassemblePacket();
    return false;
}

void CorruptRateErrorModel::SetCorruptRate(int rate_in) {
    rate = rate_in;
}
