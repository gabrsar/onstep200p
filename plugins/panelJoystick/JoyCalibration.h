#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// OnStep's global ADC resolution is not necessarily Arduino's 12-bit default.
inline int joyAdc12(int value,int range) {
  if (value<=0 || range<=0) return 0;
  if (value>=range) return 4095;
  return int(int32_t(value)*4095/range);
}

struct JoyProfile {
  uint32_t version=4;
  int16_t low[2]={0,0}, center[2]={2048,2048}, high[2]={4095,4095};
  uint8_t xAxis=1;
  int8_t xSign=1,ySign=1;
  int16_t dead[2]={0,0};
  uint8_t reserved=0;
  uint32_t checksum=0;
  bool rangesValid() const {
    if (version!=4 || xAxis!=1 || abs(xSign)!=1 || abs(ySign)!=1) return false;
    for (int i=0;i<2;++i)
      if (low[i]<0 || high[i]>4095 || center[i]-low[i]<160 || high[i]-center[i]<160 || dead[i]<0 || dead[i]>=center[i]-low[i] || dead[i]>=high[i]-center[i]) return false;
    return true;
  }
  uint32_t hash() const {
    uint32_t h=2166136261u;
    const int values[]={int(version),low[0],low[1],center[0],center[1],high[0],high[1],xAxis,xSign,ySign,dead[0],dead[1]};
    for (int v:values) h=(h^uint32_t(v))*16777619u;
    return h;
  }
  bool valid() const { return rangesValid() && checksum==hash(); }
  int scale(int raw,int axis) const {
    const int delta=raw-center[axis];
    const int span=delta<0?center[axis]-low[axis]:high[axis]-center[axis];
    if (span<=dead[axis] || abs(delta)<=dead[axis]) return 0;
    const int v=(delta<0?delta+dead[axis]:delta-dead[axis])*2048/(span-dead[axis]);
    return v<-2048?-2048:v>2048?2048:v;
  }
  void normalize(int a,int b,int &x,int &y) const {
    const int raw[]={a,b};
    x=xSign*scale(raw[xAxis],xAxis);
    y=ySign*scale(raw[1-xAxis],1-xAxis);
  }
};

class Median5 {
public:
  int update(int value) {
    if (!ready) { for (int &v:values) v=value; ready=true; }
    values[index]=value; index=(index+1)%5;
    int sorted[5]; for (int i=0;i<5;++i) sorted[i]=values[i];
    for (int i=1;i<5;++i) for (int j=i;j>0 && sorted[j]<sorted[j-1];--j) {
      const int t=sorted[j]; sorted[j]=sorted[j-1]; sorted[j-1]=t;
    }
    return sorted[2];
  }
private:
  int values[5]={}; uint8_t index=0; bool ready=false;
};

enum class JoyCalStage : uint8_t { Idle, Release, Reference, Sweep, ConfirmCenter, Center, Save };
class JoyCalibration {
public:
  JoyCalStage stage=JoyCalStage::Idle;
  JoyProfile candidate;
  const char *notice="";
  uint8_t laps=0;
  bool active() const { return stage!=JoyCalStage::Idle; }
  unsigned centerSeconds(uint32_t now) const {
    return stage==JoyCalStage::Center && collecting ? unsigned((now-phaseAt)/1000) : 0;
  }
  void begin(uint32_t now,int8_t horizontalSign=1,int8_t verticalSign=1) {
    *this=JoyCalibration();
    candidate.xSign=horizontalSign; candidate.ySign=verticalSign;
    candidate.low[0]=candidate.low[1]=candidate.high[0]=candidate.high[1]=-1;
    stage=JoyCalStage::Release; started=now;
  }
  void finish() { stage=JoyCalStage::Idle; }
  void saveFailed(uint32_t now) { stage=JoyCalStage::Save; phaseAt=now; notice="SAVE FAILED - RETRY"; }
  // Filtered samples measure travel; unfiltered samples measure resting noise.
  bool update(int a,int b,bool down,bool click,uint32_t now,int noisyA=-1,int noisyB=-1) {
    if (!active()) return false;
    if (now-started>300000) { finish(); notice="CAL CANCELLED"; return false; }
    if (stage==JoyCalStage::Save) {
      if (down || now-phaseAt<1000) return false;
      phaseAt=now; return true;
    }
    if (stage==JoyCalStage::Release) {
      if (!down) { stage=JoyCalStage::Reference; phaseAt=now; }
      return false;
    }
    const int raw[]={a,b};
    if (stage==JoyCalStage::Reference) {
      if (down) { samples=0; phaseAt=now; return false; }
      if (now-phaseAt<300) return false;
      if (!samples) sum[0]=sum[1]=0;
      for (int i=0;i<2;++i) sum[i]+=raw[i];
      if (++samples<32) return false;
      for (int i=0;i<2;++i) candidate.center[i]=sum[i]/samples;
      stage=JoyCalStage::Sweep; samples=0;
      return false;
    }
    if (stage==JoyCalStage::Sweep) {
      for (int i=0;i<2;++i) {
        if (candidate.low[i]<0 || raw[i]<candidate.low[i]) candidate.low[i]=raw[i];
        if (candidate.high[i]<0 || raw[i]>candidate.high[i]) candidate.high[i]=raw[i];
      }
      int normalized[2];
      for (int i=0;i<2;++i) {
        const int delta=raw[i]-candidate.center[i];
        int span=delta<0?candidate.center[i]-candidate.low[i]:candidate.high[i]-candidate.center[i];
        if (span<160) span=160;
        normalized[i]=delta*2048/span;
      }
      const int x=normalized[1],y=normalized[0];
      // Center motion cannot count as a turn.
      if (abs(x)<1400 && abs(y)<1400) return false;
      int sector=int(floor(atan2(double(y),double(x))*4.0/3.14159265358979323846+0.5));
      sector=(sector+8)%8;
      if (lastSector<0) { lastSector=sector; mask=uint8_t(1u<<sector); lapAt=now; return false; }
      if (sector==lastSector) return false;
      int step=(sector-lastSector+8)%8;
      if (step==1 || step==7) {
        steps+=step==1?1:-1; mask|=uint8_t(1u<<sector);
      } else { steps=0; mask=uint8_t(1u<<sector); lapAt=now; }
      lastSector=sector;
      if (abs(steps)>=8 && mask==255 && now-lapAt>=800 && candidate.rangesValid()) {
        ++laps; steps=0; mask=uint8_t(1u<<sector); lapAt=now;
        if (laps==3) { stage=JoyCalStage::ConfirmCenter; collecting=false; notice="CLICK THEN CENTER"; }
      }
      return false;
    }
    if (stage==JoyCalStage::ConfirmCenter) {
      if (click) centerClicked=true;
      if (down) phaseAt=now;
      if (centerClicked && !down && now-phaseAt>=300) {
        stage=JoyCalStage::Center; collecting=false; notice="FINGER AT CENTER";
      }
      return false;
    }
    if (stage==JoyCalStage::Center) {
      bool near=true;
      for (int i=0;i<2;++i) {
        const int delta=raw[i]-candidate.center[i];
        const int span=delta<0?candidate.center[i]-candidate.low[i]:candidate.high[i]-candidate.center[i];
        if (abs(delta)>span*30/100) near=false;
      }
      if (!near || down) { collecting=false; notice="FINGER AT CENTER"; return false; }
      const int noisy[]={noisyA<0?a:noisyA,noisyB<0?b:noisyB};
      if (!collecting) {
        collecting=true; phaseAt=now; samples=0;
        for (int i=0;i<2;++i) { sum[i]=0; noiseLow[i]=noiseHigh[i]=noisy[i]; }
      }
      for (int i=0;i<2;++i) {
        sum[i]+=noisy[i];
        if (noisy[i]<noiseLow[i]) noiseLow[i]=noisy[i];
        if (noisy[i]>noiseHigh[i]) noiseHigh[i]=noisy[i];
      }
      ++samples; notice="";
      if (now-phaseAt<3000) return false;
      for (int i=0;i<2;++i) {
        candidate.center[i]=sum[i]/samples;
        int radius=abs(noiseLow[i]-candidate.center[i]);
        const int upper=abs(noiseHigh[i]-candidate.center[i]);
        if (upper>radius) radius=upper;
        candidate.dead[i]=(radius*110+99)/100; // Round up: measured radius +10%.
        if (candidate.dead[i]<4) candidate.dead[i]=4; // One native 10-bit ADC step.
      }
      if (!candidate.rangesValid()) { finish(); notice="CENTER RANGE INVALID"; return false; }
      candidate.checksum=candidate.hash(); stage=JoyCalStage::Save; return true;
    }
    return false;
  }
private:
  uint32_t started=0,phaseAt=0,lapAt=0,samples=0;
  int32_t sum[2]={};
  int noiseLow[2]={},noiseHigh[2]={},lastSector=-1,steps=0;
  uint8_t mask=0;
  bool collecting=false,centerClicked=false;
};
