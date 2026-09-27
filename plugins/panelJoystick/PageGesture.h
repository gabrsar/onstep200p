#pragma once
#include <stdint.h>
#include <stdlib.h>

// One page per deliberate deflection. Holding, diagonal changes and threshold
// noise cannot repeat; both axes must return to center to rearm.
class PageGesture {
public:
  void reset() { armed=false; centered=false; candidate=0; }
  int8_t update(int dx, int dy, uint32_t now,int enter=700,int exit=450) {
    if (abs(dx)<=exit && abs(dy)<=exit) {
      candidate=0;
      if (!centered) { centered=true; centerAt=now; }
      if (now-centerAt>=60) armed=true;
      return 0;
    }
    centered=false;
    if (!armed) return 0;
    const int dominant=abs(dx)>=abs(dy)?dx:dy;
    const int8_t direction=dominant>=enter?1:dominant<=-enter?-1:0;
    if (direction!=candidate) { candidate=direction; candidateAt=now; }
    if (!direction || now-candidateAt<30) return 0;
    armed=false;
    candidate=0;
    return direction;
  }
private:
  bool armed=false, centered=false;
  int8_t candidate=0;
  uint32_t centerAt=0, candidateAt=0;
};
