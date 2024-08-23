#include "drop-rate-error-model.h"
#include "../helper/quic-packet.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(DropRateErrorModel);

TypeId DropRateErrorModel::GetTypeId(void)
{
    static TypeId tid = TypeId("DropRateErrorModel")
                            .SetParent<ErrorModel>()
                            .AddConstructor<DropRateErrorModel>();
    return tid;
}

DropRateErrorModel::DropRateErrorModel()
{
    rate = 0;
    burst = 0;
    dropped_in_a_row = 0;
    dropped = 0;
    forwarded = 0;
}

DropRateErrorModel::~DropRateErrorModel()
{
    cout << "Dropped " << dropped << " packets, forwarded " << forwarded
         << " packets (" << (double)dropped / (dropped + forwarded) * 100
         << "%)." << endl
         << flush;
}

void DropRateErrorModel::DoReset(void)
{
    dropped_in_a_row = 0;
    dropped = 0;
    forwarded = 0;
}

bool DropRateErrorModel::DoCorrupt(Ptr<Packet> p)
{
    if (!IsUDPPacket(p))
        return false;

    bool shouldDrop = false;
    if (dropped_in_a_row >= burst) {
        dropped_in_a_row = 0;
        shouldDrop = false;
    } else if (std::rand() % 100 < rate) {
        dropped_in_a_row++;
        shouldDrop = true;
    } else {
        dropped_in_a_row = 0;
        shouldDrop = false;
    }

    QuicPacket qp = QuicPacket(p);

    if (shouldDrop) {
        cout << "Dropping packet (" << qp.GetUdpPayload().size()
             << " bytes) from " << qp.GetIpv4Header().GetSource() << ":"
             << qp.GetUdpHeader().GetSourcePort() << " to "
             << qp.GetIpv4Header().GetDestination() << ":"
             << qp.GetUdpHeader().GetDestinationPort() << endl
             << flush;
        dropped++;
    } else {
        cout << "Forwarding packet (" << qp.GetUdpPayload().size()
             << " bytes) from " << qp.GetIpv4Header().GetSource() << ":"
             << qp.GetUdpHeader().GetSourcePort() << " to "
             << qp.GetIpv4Header().GetDestination() << ":"
             << qp.GetUdpHeader().GetDestinationPort() << endl
             << flush;
        forwarded++;
        qp.ReassemblePacket();
    }
    return shouldDrop;
}

void DropRateErrorModel::SetDropRate(int rate_in)
{
    rate = rate_in;
}

void DropRateErrorModel::SetMaxDropBurst(int burst_in)
{
    burst = burst_in;
}
