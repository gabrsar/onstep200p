#pragma once
#include <stdint.h>

class CenterCalibration {
public:
  bool add(int x,int y) {
    sumX+=x; sumY+=y;
    if (x<minX) minX=x;
    if (x>maxX) maxX=x;
    if (y<minY) minY=y;
    if (y>maxY) maxY=y;
    return ++samples==32;
  }
  bool valid() const {
    return samples==32 && minX>=400 && maxX<=3695 && minY>=400 && maxY<=3695 &&
           maxX-minX<=250 && maxY-minY<=250;
  }
  int x() const { return sumX/32; }
  int y() const { return sumY/32; }
  uint8_t remaining() const { return 32-samples; }
private:
  uint32_t sumX=0,sumY=0;
  int minX=4095,minY=4095,maxX=0,maxY=0;
  uint8_t samples=0;
};
