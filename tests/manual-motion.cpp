#include "../plugins/panelJoystick/ManualMotion.h"
#include "../plugins/panelJoystick/CenterCalibration.h"
#include <cassert>
#include <string>
#include <vector>
struct Bus {
  int capacity=8;
  std::vector<std::string> sent;
  bool send(const char *s) {
    if (!capacity) return false;
    --capacity; sent.push_back(s); return true;
  }
};
int main() {
  Bus bus;
  ManualMotion m;
  assert(m.update(bus,1,0));
  assert(bus.sent[0]==":RS#" && bus.sent[1]==":Mw#");
  assert(m.update(bus,-1,0));
  assert(bus.sent[2]==":Qe#" && bus.sent[4]==":Me#");
  bus.capacity=0;
  assert(!m.update(bus,0,0));
  assert(m.pendingStop());
  assert(!m.update(bus,1,1));
  bus.capacity=1;
  assert(m.serviceStop(bus));
  assert(bus.sent.back()==":Q#");
  // Rate accepted but movement rejected must fall back to STOP retry.
  bus.capacity=1;
  assert(!m.update(bus,1,0));
  assert(m.pendingStop() && m.direction(0)==0);
  CenterCalibration stable,rail,moving;
  for (int i=0;i<32;++i) {
    stable.add(2048+i%10,2000);
    rail.add(0,4095);
    moving.add(i%2?1600:2400,2048);
  }
  assert(stable.valid() && stable.remaining()==0);
  assert(!rail.valid() && !moving.valid());
}
