#include "../plugins/panelJoystick/PageGesture.h"
#include <cassert>
int main() {
  PageGesture g;
  assert(g.update(2000,0,0)==0); // Must center after entering UI.
  g.update(0,0,10); g.update(0,0,70);
  assert(g.update(710,0,80)==0);
  assert(g.update(690,0,100)==0); // Threshold noise resets debounce.
  assert(g.update(800,0,110)==0);
  assert(g.update(800,0,140)==1);
  assert(g.update(2000,0,1000)==0); // Holding never repeats.
  assert(g.update(-1000,2000,1100)==0); // Diagonal/sign change cannot repeat.
  g.update(0,0,1200);
  assert(g.update(-1000,0,1240)==0); // Too brief a center does not rearm.
  g.update(0,0,1300); g.update(0,0,1360);
  g.update(-1000,0,1370);
  assert(g.update(-1000,0,1400)==-1);
  g.reset();
  assert(g.update(-1000,0,2000)==0);
  // Calibrated input already has its measured deadzone removed.
  g.update(0,0,2010,1,0); g.update(0,0,2070,1,0);
  assert(g.update(10,0,2080,1,0)==0);
  assert(g.update(10,0,2110,1,0)==1);
  assert(g.update(500,0,2120,1,0)==0);
}
