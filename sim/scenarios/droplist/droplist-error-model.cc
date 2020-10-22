#include "../helper/quic-packet.h"
#include "droplist-error-model.h"

using namespace std;

NS_OBJECT_ENSURE_REGISTERED(DroplistErrorModel);
 
TypeId DroplistErrorModel::GetTypeId(void) {
    static TypeId tid = TypeId("DroplistErrorModel")
        .SetParent<ErrorModel>()
        .AddConstructor<DroplistErrorModel>()
        ;
    return tid;
}
 
DroplistErrorModel::DroplistErrorModel()
    : packet_num(0) { }

void DroplistErrorModel::DoReset(void) { }
 
bool DroplistErrorModel::DoCorrupt(Ptr<Packet> p) {
    if(!IsUDPPacket(p)) return false;
    if(drops.find(++packet_num) == drops.end()) return false;
    
    QuicPacket qp = QuicPacket(p);
    cout << "Dropping packet " << packet_num << " (" << qp.GetUdpPayload().size() << " bytes) from " << qp.GetIpv4Header().GetSource() << endl;
    qp.ReassemblePacket();
    return true;
}

void DroplistErrorModel::SetDrop(int packet_num) {
    drops.insert(packet_num);
}
