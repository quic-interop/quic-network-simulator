#ifndef QUIC_PACKET_H
#define QUIC_PACKET_H

#include <cstdint>

#include "ns3/header.h"
#include "ns3/packet.h"
#include "ns3/ppp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/udp-header.h"

using namespace ns3;
using namespace std;

bool IsUDPPacket(Ptr<Packet> p);

class QuicPacket {
public:
    QuicPacket(Ptr<Packet> p);
    // Assemble a new packet.
    // It uses the PPP, IP and UDP header that it parsed from the original packet,
    // and recalculates IP and UDP checksums.
    void ReassemblePacket();
    Ipv4Header& GetIpv4Header();
    UdpHeader& GetUdpHeader();
    vector<uint8_t>& GetUdpPayload();
    bool IsVersionNegotiationPacket();

private:
    Ptr<Packet> p_;
    PppHeader ppp_hdr_;
    Ipv4Header ipv4_hdr_;
    UdpHeader udp_hdr_;
    uint32_t udp_hdr_len_;
    uint32_t total_hdr_len_;
    vector<uint8_t> udp_payload_;
};

#endif /* QUIC_PACKET_H */
