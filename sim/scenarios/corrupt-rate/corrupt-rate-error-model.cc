#include <iomanip>

#include <cstdint>
#include <vector>

#include "../helper/quic-packet.h"
#include "corrupt-rate-error-model.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(CorruptRateErrorModel);

TypeId CorruptRateErrorModel::GetTypeId(void)
{
    static TypeId tid = TypeId("CorruptRateErrorModel")
                            .SetParent<ErrorModel>()
                            .AddConstructor<CorruptRateErrorModel>();
    return tid;
}

CorruptRateErrorModel::CorruptRateErrorModel()
    : rate(0), distr(0, 99), burst(INT_MAX), corrupted_in_a_row(0),
      corrupted(0), forwarded(0)
{
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void CorruptRateErrorModel::DoReset(void)
{
    corrupted_in_a_row = 0;
    corrupted = 0;
    forwarded = 0;
}

bool CorruptRateErrorModel::DoCorrupt(Ptr<Packet> p)
{
    if (!IsUDPPacket(p))
        return false;

    QuicPacket qp = QuicPacket(p);

    bool shouldCorrupt = false;
    if (qp.IsVersionNegotiationPacket()) {
        // Don't corrupt Version Negotiation packets.
        // Version Negotiation packets are expected to be sent when the version
        // field of the Initial was corrupted. Client are supposed to ignore
        // Version Negotiation packets that contain the version that they
        // offered.
        corrupted_in_a_row = 0;
        shouldCorrupt = false;
    } else if (corrupted_in_a_row >= burst) {
        corrupted_in_a_row = 0;
        shouldCorrupt = false;
    } else if (distr(*rng) < rate) {
        corrupted_in_a_row++;
        shouldCorrupt = true;
    } else {
        corrupted_in_a_row = 0;
        shouldCorrupt = false;
    }

    if (shouldCorrupt) {
        cout << "Corrupting ";
        corrupted++;

        // Corrupt a byte in the 50 bytes of the UDP payload.
        // This way, we will frequenetly hit the QUIC header.
        std::uniform_int_distribution<> d(0,
                                          min(uint32_t(50), p->GetSize() - 1));
        int pos = d(*rng);
        vector<uint8_t> & payload = qp.GetUdpPayload();
        // Replace the byte at position pos with a random value.
        while (true) {
            uint8_t n = std::uniform_int_distribution<>(0, 255)(*rng);
            if (payload[pos] == n)
                continue;
            cout << "Corrupted packet (" << qp.GetUdpPayload().size()
                 << " bytes) from " << qp.GetIpv4Header().GetSource()
                 << " at offset " << pos << " (0x" << std::hex
                 << (unsigned int)payload[pos] << " -> 0x" << (unsigned int)n
                 << ")" << std::dec << endl;
            payload[pos] = n;
            break;
        }
        qp.ReassemblePacket();
    } else {
        cout << "Forwarding ";
        forwarded++;
        qp.ReassemblePacket();
    }
    cout << qp.GetUdpPayload().size() << " bytes "
         << qp.GetIpv4Header().GetSource() << ":"
         << qp.GetUdpHeader().GetSourcePort() << " -> "
         << qp.GetIpv4Header().GetDestination() << ":"
         << qp.GetUdpHeader().GetDestinationPort() << ", corrupted "
         << corrupted << "/" << corrupted + forwarded << " (" << fixed
         << setprecision(1) << (double)corrupted / (corrupted + forwarded) * 100
         << "%)" << endl;

    return shouldCorrupt;
}

void CorruptRateErrorModel::SetCorruptRate(int rate_in)
{
    rate = rate_in;
}
