#include "../plugins/panelJoystick/ClickGesture.h"
#include <cassert>
#include <cstdint>

static bool press(ClickGesture &g, uint32_t t) {
  assert(!g.update(true,t));
  bool toggled=g.update(true,t+40);
  assert(g.blocked());
  assert(!g.update(false,t+80));
  assert(!g.update(false,t+120));
  return toggled;
}
int main() {
  ClickGesture g;
  assert(!g.blocked());
  assert(!g.takeSingle());
  assert(!press(g,100));
  assert(!press(g,250));
  assert(press(g,400));
  assert(!g.blocked());
  assert(!g.takeSingle()); // Triple click never advances a page as a single.
  assert(!press(g,600));
  g.update(false,1200);
  assert(!g.blocked());
  assert(g.takeSingle());
  assert(!g.takeSingle());
  assert(!press(g,1300));
  // Bounce must not count as extra clicks.
  g.update(true,1500); g.update(false,1510); g.update(true,1520);
  assert(!g.update(true,1560));
  assert(!g.update(true,2200));
  assert(g.blocked());
  ClickGesture wrap;
  assert(!press(wrap,UINT32_MAX-200));
  assert(!press(wrap,UINT32_MAX-50));
  assert(press(wrap,100));
  assert(!wrap.takeSingle());

  ClickGesture one;
  press(one,100); assert(!one.takeSingle());
  one.update(false,580); assert(!one.takeSingle());
  one.update(false,600); assert(one.takeSingle());
  for (uint32_t t=700;t<2000;t+=10) { one.update(false,t); assert(!one.takeSingle()); }

  ClickGesture held;
  held.update(true,100); held.update(true,140);
  held.update(true,2000); assert(!held.takeSingle());
  held.update(false,2100); held.update(false,2140); held.update(false,3000);
  assert(!held.takeSingle());

  ClickGesture two;
  press(two,100); press(two,250); two.update(false,1000);
  assert(!two.takeSingle());

  ClickGesture singleWrap;
  press(singleWrap,UINT32_MAX-200); singleWrap.update(false,500);
  assert(singleWrap.takeSingle());
}
