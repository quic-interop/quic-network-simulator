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
    :
        rate(0),
        distr(0, 99),
        drop_counter(0),
        pass_counter(0),
        first_packet_logged(false)
{
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void DropRateErrorModel::DoReset(void) { }

void DropRateErrorModel::log(void) {
    if (!first_packet_logged) {
        if (pass_counter > 0) {
            cout << "First packet forwarded." << endl;
        } else if (drop_counter > 0) {
            cout << "First packet dropped." << endl;
        } else {
            cout << "WTF!" << endl;
        }
        first_packet_logged = true;
        cout << "Will log stats after every " << LOG_AFTER_PACKETS << "th packet." << endl;
    } else if (drop_counter + pass_counter >= LOG_AFTER_PACKETS) {
        cout << "Among the last " << LOG_AFTER_PACKETS << " packets, " << drop_counter << " packets were dropped and " << pass_counter << " were forwarded." << endl;
        drop_counter = 0;
        pass_counter = 0;
    }
}

bool DropRateErrorModel::DoCorrupt(Ptr<Packet> p) {
    if (!IsUDPPacket(p)) {
        return false;
    }

    QuicPacket qp = QuicPacket(p);

    if (distr(*rng) >= rate) {
        // cout << "Forwarding packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
        // if (rate > 0) {
        pass_counter += 1;
        log();
        // }
        qp.ReassemblePacket();
        return false;
    }

    // cout << "Dropping packet (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
    // if (rate > 0) {
    drop_counter += 1;
    log();
    // }
    return true;
}

void DropRateErrorModel::SetDropRate(int rate_in) {
    rate = rate_in;
}
