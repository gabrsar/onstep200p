#pragma once
#include "../panelJoystick/JoyCalibration.h"

// Diagnostic pitch uses measured ADC displacement, not the uncalibrated
// orientation or the movement threshold. Short pulses limit duty cycle.
inline unsigned calibrationTone(const JoyCalibration &cal,int a,int b,uint32_t now,bool enabled) {
  if (!enabled || cal.stage!=JoyCalStage::Sweep) return 0;
  const int delta[]={a-cal.candidate.center[0],b-cal.candidate.center[1]};
  const int axis=abs(delta[0])>=abs(delta[1])?0:1;
  const int value=delta[axis];
  if (abs(value)<80 || now%200>=60) return 0;
  const int quantized=(value/32)*32;
  const int hz=1200+quantized*800/2048;
  return hz<400?400:hz>2200?2200:unsigned(hz);
}
