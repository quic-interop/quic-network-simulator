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
    : rate(0), consecutivelyCorrupted(0), distr(0, 99) {
    std::random_device rd;
    rng = new std::mt19937(rd());
}

void DropRateErrorModel::DoReset(void) {
    consecutivelyCorrupted = 0;
}

bool DropRateErrorModel::DoCorrupt(Ptr<Packet> p) {
    if (consecutivelyCorrupted >= 3 || distr(*rng) >= rate) {
        consecutivelyCorrupted = 0;
        return false;
    }
    consecutivelyCorrupted++;
    cout << "Dropping packet" << endl;
    return true;
}

void DropRateErrorModel::SetDropRate(int rate_in) {
    rate = rate_in;
}
