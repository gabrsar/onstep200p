#include "../plugins/panelJoystick/CenterCalibration.h"
#include <cassert>
int main() {
  CenterCalibration shortAxis;
  for (int i=0;i<32;++i) shortAxis.add(i%2?950:970,2000);
  assert(shortAxis.valid() && shortAxis.x()==960 && shortAxis.y()==2000);
  CenterCalibration rail;
  for (int i=0;i<32;++i) rail.add(0,2000);
  assert(!rail.valid());
  CenterCalibration moving;
  for (int i=0;i<32;++i) moving.add(i%2?700:1950,2000);
  assert(!moving.valid());
}
