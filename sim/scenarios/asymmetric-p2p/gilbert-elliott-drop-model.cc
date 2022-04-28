#include "../helper/quic-packet.h"
#include "gilbert-elliott-drop-model.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(GilbertElliottDropModel);

TypeId GilbertElliottDropModel::GetTypeId(void) {
    static TypeId tid = TypeId("GilbertElliottDropModel")
        .SetParent<ErrorModel>()
        .AddConstructor<GilbertElliottDropModel>()
        ;
    return tid;
}

GilbertElliottDropModel::GilbertElliottDropModel()
    :
        goodBadPerc(0),
        badGoodPerc(0),
        state(GilbertElliottState::GOOD),
        distr(0.0, 1.0),
        drop_counter(0),
        pass_counter(0),
        first_packet_logged(false)
{
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void GilbertElliottDropModel::DoReset(void) { }

void GilbertElliottDropModel::log(void) {
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
        cout << "Among the last " <<
            LOG_AFTER_PACKETS << " packets, " <<
            drop_counter << " packets were dropped and " <<
            pass_counter << " were forwarded -> p_err = " <<
            drop_counter * 100.0 / LOG_AFTER_PACKETS << " %" <<
            endl;
        drop_counter = 0;
        pass_counter = 0;
    }
}

bool GilbertElliottDropModel::DoCorrupt(Ptr<Packet> p) {
    if (!IsUDPPacket(p)) {
        return false;
    }

    QuicPacket qp = QuicPacket(p);

    // check if we switch the state:
    float rnd = distr(*rng);
    if (this->state == GilbertElliottState::GOOD && rnd < this->goodBadPerc) {
        state = GilbertElliottState::BAD;
    } else if (this->state == GilbertElliottState::BAD && rnd < this->badGoodPerc) {
        state = GilbertElliottState::GOOD;
    }

    if (this->state == GilbertElliottState::GOOD) {
        pass_counter += 1;
        log();
        qp.ReassemblePacket();
        return false;
    } else {
        drop_counter += 1;
        log();
        return true;
    }

}

void GilbertElliottDropModel::SetProbabilities(float goodBadPerc, float badGoodPerc) {
    this->goodBadPerc = goodBadPerc;
    this->badGoodPerc = badGoodPerc;
}
