#include "../helper/quic-packet.h"
#include "drop-rate-error-model.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(DropRateErrorModel);

TypeId DropRateErrorModel::GetTypeId(void) {
    static TypeId tid = TypeId("DropRateErrorModel")
        .SetParent<ErrorModel>()
        .AddConstructor<DropRateErrorModel>()
        ;
    return tid;
}
 
DropRateErrorModel::DropRateErrorModel()
    : rate(0), distr(0, 99) {
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void DropRateErrorModel::DoReset(void) { }
 
bool DropRateErrorModel::DoCorrupt(Ptr<Packet> p) {
    if(!IsUDPPacket(p)) return false;

    QuicPacket qp = QuicPacket(p);

    if (distr(*rng) >= rate) {
        cout << "Forwarding packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
        qp.ReassemblePacket();
        return false;
    }

    cout << "Dropping packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
    return true;
}

void DropRateErrorModel::SetDropRate(int rate_in) {
    rate = rate_in;
}
