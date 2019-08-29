#include <map>
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
  return drops.empty() ? 0 : drops.find(++packet_num) != drops.end();
}

void DroplistErrorModel::SetDrops(string drops_in) {
  char *cstr = new char[drops_in.length()+1];
  strcpy(cstr, drops_in.c_str());
  char *p = strtok(cstr,",");
  while (p) {
    drops.insert(make_pair(stoi(p), true));
    p = strtok(NULL,",");
  }
  delete[] cstr;
}

