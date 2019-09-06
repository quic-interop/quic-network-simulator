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
 
DroplistErrorModel::DroplistErrorModel() { }

void DroplistErrorModel::DoReset(void) { }
 
bool DroplistErrorModel::DoCorrupt(Ptr<Packet> p) {
    static int packet_num = 0;
    if(drops.find(++packet_num) == drops.end())
        return false;
    cout << "Dropping packet number " << packet_num << endl;
    return true;
}

void DroplistErrorModel::SetDrop(int packet_num) {
    drops.insert(packet_num);
}
