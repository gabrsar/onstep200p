#include "../plugins/panelJoystick/JoyCalibration.h"
#include "../plugins/panelDisplay/JoyArrows.h"
#include <cassert>
#include <cstring>
#include <initializer_list>

int main() {
  assert(joyAdc12(0,1023)==0 && joyAdc12(1023,1023)==4095);
  assert(joyAdc12(512,1023)>=2048 && joyAdc12(512,1023)<=2050);
  JoyProfile p; p.checksum=p.hash(); assert(p.valid() && p.xAxis==1);
  int x,y; p.normalize(0,4095,x,y); assert(x==2048 && y==-2048);
  p.version=3; p.checksum=p.hash(); assert(!p.valid());
  Median5 filter;
  for (int i=0;i<10;++i) assert(filter.update(2000)==2000);
  assert(filter.update(4095)==2000); assert(filter.update(0)==2000);

  JoyCalibration cal; uint32_t now=0;
  auto run=[&](int a,int b,int n) {
    bool saved=false;
    for (int i=0;i<n;++i) { now+=10; saved=cal.update(a,b,false,false,now)||saved; }
    return saved;
  };
  cal.begin(now);
  assert(cal.candidate.low[0]==-1 && cal.candidate.high[1]==-1);
  run(1000,2100,100); assert(cal.stage==JoyCalStage::Sweep);
  assert(cal.candidate.low[0]==1000 || cal.candidate.low[0]==-1);
  // Three circles without clicks; one confirmation before center capture.
  const int ring[8][2]={{1000,4095},{2100,4095},{2100,2100},{2100,0},
                        {1000,0},{0,0},{0,2100},{0,4095}};
  run(1000,4095,15);
  for (int lap=0;lap<3;++lap) {
    for (int i=1;i<=8;++i) run(ring[i%8][0],ring[i%8][1],15);
    assert(cal.laps==lap+1);
  }
  assert(cal.stage==JoyCalStage::ConfirmCenter);
  run(1000,2100,400); assert(cal.stage==JoyCalStage::ConfirmCenter);
  cal.update(1000,2100,true,true,++now);
  run(1000,2100,31); assert(cal.stage==JoyCalStage::Center);
  assert(cal.candidate.low[0]==0 && cal.candidate.high[0]==2100);
  assert(cal.candidate.low[1]==0 && cal.candidate.high[1]==4095);
  // Leaving center restarts the full three-second measurement.
  run(1000,2100,100); run(2100,2100,1);
  assert(cal.centerSeconds(now)==0);
  bool saved=false;
  for (int i=0;i<301;++i) {
    now+=10;
    saved=cal.update(1000,2100,false,false,now,i%2?980:1020,i%2?2090:2110)||saved;
  }
  assert(saved && cal.stage==JoyCalStage::Save && cal.candidate.valid());
  assert(cal.candidate.dead[0]>=22 && cal.candidate.dead[1]>=11);
  cal.candidate.normalize(1020,2110,x,y); assert(x==0 && y==0);
  cal.candidate.normalize(0,4095,x,y); assert(x==2048 && y==-2048);
  cal.saveFailed(now); assert(!cal.update(1000,2100,false,false,++now));
  now+=1000; assert(cal.update(1000,2100,false,false,now));
  cal.finish(); assert(!cal.active());

  // Back-and-forth and center noise never count as revolutions.
  now=0; cal.begin(now); run(1000,2100,100);
  for (int i=0;i<50;++i) { run(1000,4095,10); run(2100,4095,10); run(1000,4095,10); }
  assert(cal.laps==0);
  cal.begin(0xfffffff0); cal.update(1000,2100,false,false,300000);
  assert(!cal.active());

  // Both clockwise and counterclockwise turns are accepted.
  now=0; cal.begin(now); run(1000,2100,100); run(ring[0][0],ring[0][1],15);
  for (int lap=0;lap<3;++lap)
    for (int i=7;i>=0;--i) run(ring[i][0],ring[i][1],15);
  assert(cal.laps==3 && cal.stage==JoyCalStage::ConfirmCenter);

  // Smooth turns, asymmetrical travel and arbitrary starting angles work too.
  for (int offset:{0,45,180,270}) {
    now=0; cal.begin(now); run(1500,2000,100);
    for (int degree=offset;degree<=offset+1100 && cal.stage==JoyCalStage::Sweep;++degree) {
      const double angle=degree*3.14159265358979323846/180.0;
      const double vertical=sin(angle),horizontal=cos(angle);
      run(int(1500+vertical*(vertical<0?1500:600)),
          int(2000+horizontal*(horizontal<0?2000:2095)),1);
    }
    assert(cal.laps==3 && cal.stage==JoyCalStage::ConfirmCenter);
  }

  // A button interruption cannot leak old reference samples into the next capture.
  now=0; cal.begin(now); run(1000,2100,40);
  cal.update(1000,2100,true,true,++now);
  run(1000,2100,100);
  assert(cal.candidate.center[0]==1000 && cal.candidate.center[1]==2100);

  uint16_t right[9],left[9],up[9],center[9];
  joyArrow(2048,0,right); joyArrow(-2048,0,left); joyArrow(0,2048,up); joyArrow(0,0,center);
  assert(memcmp(right,center,sizeof(right))!=0);
  joyArrow(1,0,left,0); assert(memcmp(left,center,sizeof(left))!=0);
  joyArrow(-2048,0,left);
  for (int row=0;row<9;++row) for (int col=0;col<9;++col) {
    assert(bool(right[row]&(1u<<col))==bool(left[8-row]&(1u<<(8-col))));
    assert(bool(right[row]&(1u<<col))==bool(up[8-col]&(1u<<row)));
  }
  for (int level:{500,900,1800}) for (int sx:{-1,1}) for (int sy:{-1,1}) {
    joyArrow(sx*level,sy*level,right);
    for (uint16_t bits:right) assert((bits&~0x1ffu)==0);
  }
}
