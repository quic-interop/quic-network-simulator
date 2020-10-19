#include <cstdint>
#include <vector>

#include "quic-packet.h"

#include "ns3/packet.h"
#include "ns3/ppp-header.h"
#include "ns3/ipv4-header.h"
#include "ns3/ipv6-header.h"
#include "ns3/udp-header.h"

using namespace ns3;
using namespace std;


bool IsUDPPacket(Ptr<Packet> p) {
    PppHeader ppp_hdr = PppHeader();
    p->RemoveHeader(ppp_hdr);
    bool is_udp = false;
    switch(ppp_hdr.GetProtocol()) {
        case 0x21: // IPv4
            {
                Ipv4Header hdr = Ipv4Header();
                p->PeekHeader(hdr);
                is_udp = hdr.GetProtocol() == 17;
            }
            break;
        case 0x57: // IPv6
            {
                Ipv6Header hdr = Ipv6Header();
                p->PeekHeader(hdr);
                is_udp = hdr.GetNextHeader() == 17;
            }
            break;
        default:
            cout << "Unknown PPP protocol: " << ppp_hdr.GetProtocol() << endl;
            break;
    }
    p->AddHeader(ppp_hdr);
    return is_udp;
}


QuicPacket::QuicPacket(Ptr<Packet> p) : p_(p) {
    const uint32_t p_len = p->GetSize();
    uint8_t *buffer= new uint8_t[p_len];
    p->CopyData(buffer, p_len);
    ppp_hdr_ = PppHeader();
    uint32_t ppp_hdr_len = p->RemoveHeader(ppp_hdr_);
    // TODO: This currently only works for IPv4.
    ipv4_hdr_ = Ipv4Header();
    uint32_t ip_hdr_len = p->RemoveHeader(ipv4_hdr_);
    udp_hdr_ = UdpHeader();
    udp_hdr_len_ = p->RemoveHeader(udp_hdr_);
    total_hdr_len_ = ppp_hdr_len + ip_hdr_len + udp_hdr_len_;
    udp_payload_ = vector<uint8_t>(&buffer[total_hdr_len_], &buffer[total_hdr_len_] + p_len - total_hdr_len_);
}

Ipv4Header& QuicPacket::GetIpv4Header() { return ipv4_hdr_; }

UdpHeader& QuicPacket::GetUdpHeader() { return udp_hdr_; }

vector<uint8_t>& QuicPacket::GetUdpPayload() { return udp_payload_; }

bool QuicPacket::IsVersionNegotiationPacket() {
    if(udp_payload_.size() <= 5) return false;
    return udp_payload_[1] == 0 && udp_payload_[2] == 0 && udp_payload_[3] == 0 && udp_payload_[4] == 0;
}

void QuicPacket::ReassemblePacket() {
    // Start with the UDP payload.
    Packet new_p = Packet(udp_payload_.data(), udp_payload_.size());
    // Add the UDP header and make sure to recalculate the checksum.
    udp_hdr_.ForcePayloadSize(udp_payload_.size() + udp_hdr_len_);
    udp_hdr_.ForceChecksum(0);
    udp_hdr_.InitializeChecksum(ipv4_hdr_.GetSource(), ipv4_hdr_.GetDestination(), ipv4_hdr_.GetProtocol());
    new_p.AddHeader(udp_hdr_);
    // Add the IP header, again make sure to recalculate the checksum.
    ipv4_hdr_.EnableChecksum();
    new_p.AddHeader(ipv4_hdr_);
    // Add the PPP header.
    new_p.AddHeader(ppp_hdr_);
    p_->RemoveAtEnd(p_->GetSize());
    p_->AddAtEnd(Ptr<Packet>(&new_p));
}
