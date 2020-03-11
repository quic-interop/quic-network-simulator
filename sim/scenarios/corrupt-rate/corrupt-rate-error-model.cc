#include <cstdint>

#include "corrupt-rate-error-model.h"
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

    const uint32_t p_len = p->GetSize();
    uint8_t *buffer= new uint8_t[p_len];
    p->CopyData(buffer, p_len);
    PppHeader ppp_hdr = PppHeader();
    uint32_t ppp_hdr_len = p->RemoveHeader(ppp_hdr);
    Ipv4Header ip_hdr = Ipv4Header();
    uint32_t ip_hdr_len = p->RemoveHeader(ip_hdr);
    UdpHeader udp_hdr = UdpHeader();
    uint32_t udp_hdr_len = p->RemoveHeader(udp_hdr);
    const uint32_t hdr_len = ppp_hdr_len + ip_hdr_len + udp_hdr_len;
    // Corrupt a byte in the 50 bytes of the UDP payload.
    // This way, we will frequenetly hit the QUIC header.
    std::uniform_int_distribution<> d(hdr_len, hdr_len + min(uint32_t(50), p->GetSize() - 1));
    auto pos = d(*rng);
    buffer[pos] ^= 0xff;
    cout << "Corrupting byte " << pos - hdr_len << " of the UDP payload (total: " << p->GetSize() << ")" << endl;

    // Re-assemble a new packet.
    uint32_t udp_payload_len = p_len - hdr_len;
    // Start with the UDP payload (including the corrupted bytes).
    Packet new_p = Packet(&buffer[hdr_len], udp_payload_len);
    // Add the UDP header and make sure to recalculate the checksum.
    udp_hdr.ForcePayloadSize(udp_payload_len + udp_hdr_len);
    udp_hdr.ForceChecksum(0);
    udp_hdr.InitializeChecksum(ip_hdr.GetSource(), ip_hdr.GetDestination(), ip_hdr.GetProtocol());
    new_p.AddHeader(udp_hdr);
    // Add the IP header, again make sure to recalculate the checksum.
    ip_hdr.EnableChecksum();
    new_p.AddHeader(ip_hdr);
    // Add the PPP header.
    new_p.AddHeader(ppp_hdr);
    p->RemoveAtEnd(p_len);
    p->AddAtEnd(Ptr<Packet>(&new_p));
    return false;
}

void CorruptRateErrorModel::SetCorruptRate(int rate_in) {
    rate = rate_in;
}
