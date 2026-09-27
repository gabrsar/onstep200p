#include "../plugins/panelJoystick/JoyIndicator.h"
#include <cassert>
#include <cstring>
int main() {
  char out[3];
  joyDirection(0,0,out); assert(!strcmp(out,". "));
  joyDirection(450,-450,out); assert(!strcmp(out,". "));
  joyDirection(-451,0,out); assert(!strcmp(out,"< "));
  joyDirection(-700,0,out); assert(!strcmp(out,"<<"));
  joyDirection(1500,300,out); assert(!strcmp(out,">>"));
  joyDirection(0,600,out); assert(!strcmp(out,"^ "));
  joyDirection(700,1000,out); assert(!strcmp(out,"^^"));
  joyDirection(0,-1000,out); assert(!strcmp(out,"vv"));
  ClickIndicator click; assert(!click.lit(0));
  click.update(true,10); assert(click.lit(1000));
  click.update(false,1000); assert(!click.lit(1000));
  click.update(true,0xfffffff0); click.update(false,0xfffffff1);
  assert(click.lit(20)); assert(!click.lit(200));
}
