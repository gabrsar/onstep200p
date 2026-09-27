#pragma once
#include <stdint.h>

// Raw input visualization, independent of calibration/motion/UI gating.
inline void joyDirection(int dx, int dy, char out[3]) {
  const int ax=dx<0?-dx:dx, ay=dy<0?-dy:dy;
  const int strength=ax>=ay?ax:ay;
  const char direction=ax>=ay?(dx<0?'<':'>'):(dy<0?'v':'^');
  out[0]=strength>450?direction:'.';
  out[1]=strength>=700?direction:' ';
  out[2]=0;
}

class ClickIndicator {
public:
  void update(bool down,uint32_t now) {
    if (down) { lastDown=now; seen=true; }
    pressed=down;
  }
  bool lit(uint32_t now) const { return pressed || (seen && now-lastDown<180); }
private:
  bool pressed=false,seen=false;
  uint32_t lastDown=0;
};
