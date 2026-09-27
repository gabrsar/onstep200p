#pragma once
#include <stdint.h>

// Directions reflect accepted queue entries, not confirmed physical motion.
// The prepared upstream broker must preserve FIFO ordering.
class ManualMotion {
public:
  int8_t direction(uint8_t axis) const { return directions[axis]; }
  bool pendingStop() const { return halt; }
  template<class Transport> bool serviceStop(Transport &bus) {
    if (halt && bus.send(":Q#")) halt=false;
    return !halt;
  }
  template<class Transport> void stop(Transport &bus) {
    directions[0]=directions[1]=0;
    halt=true;
    serviceStop(bus);
  }
  template<class Transport> bool update(Transport &bus, int8_t x, int8_t y) {
    if (halt) { serviceStop(bus); return false; }
    const int8_t wanted[2]={x,y};
    for (uint8_t axis=0; axis<2; ++axis) {
      if (wanted[axis]==directions[axis]) continue;
      if (directions[axis] && !bus.send(axis==0?":Qe#":":Qn#")) {
        stop(bus); return false;
      }
      directions[axis]=0;
      if (wanted[axis] && !start(bus,axis,wanted[axis])) {
        stop(bus); return false;
      }
      directions[axis]=wanted[axis];
    }
    return true;
  }
  template<class Transport> bool refresh(Transport &bus) {
    if (halt) return false;
    for (uint8_t axis=0; axis<2; ++axis)
      if (directions[axis] && !start(bus,axis,directions[axis])) {
        stop(bus); return false;
      }
    return true;
  }
private:
  template<class Transport> bool start(Transport &bus,uint8_t axis,int8_t dir) {
    if (!bus.send(":RS#")) return false;
    return bus.send(axis==0?(dir>0?":Mw#":":Me#"):(dir>0?":Mn#":":Ms#"));
  }
  int8_t directions[2]={0,0};
  bool halt=false;
};
