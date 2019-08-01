#include "blackhole-error-model.h"
 
NS_OBJECT_ENSURE_REGISTERED(BlackholeErrorModel);
 
TypeId BlackholeErrorModel::GetTypeId(void) {
  static TypeId tid = TypeId("BlackholeErrorModel")
    .SetParent<ErrorModel>()
    .AddConstructor<BlackholeErrorModel>()
    ;
  return tid;
}
 
BlackholeErrorModel::BlackholeErrorModel() : enabled_(true) { }
 
bool BlackholeErrorModel::DoCorrupt(Ptr<Packet> p) {
  return enabled_;
}

void BlackholeErrorModel::Enable() {
  enabled_ = true;
}

void BlackholeErrorModel::Disable() {
  enabled_ = false;
}

void BlackholeErrorModel::DoReset(void) { }
