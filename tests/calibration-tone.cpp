#include "../plugins/panelFeedback/CalibrationTone.h"
#include <cassert>
#include "../plugins/panelFeedback/BeepEvents.h"
#include "../plugins/panelFeedback/BootMelody.h"
#include "../plugins/panelFeedback/ErrorMelody.h"
int main() {
  assert(beepPulses(BeepEvent::Boot)==1);
  assert(beepPulses(BeepEvent::Ready)==2);
  assert(beepPulses(BeepEvent::TimeReady)==1);
  assert(beepPulses(BeepEvent::CalStep)==1 && beepPulses(BeepEvent::CalDone)==2);
  JoyCalibration cal;
  assert(calibrationTone(cal,0,0,0,true)==0);
  assert(bootMelodyTone(0)==523 && bootMelodyTone(200)==392);
  assert(bootMelodyTone(300)==392 && bootMelodyTone(400)==440);
  assert(bootMelodyTone(600)==392 && bootMelodyTone(800)==0);
  assert(readyMelodyTone(0)==494 && readyMelodyTone(200)==523);
  assert(readyMelodyTone(175)==0 && readyMelodyTone(460)==0);
  assert(errorMelodyTone(0)==784 && errorMelodyTone(180)==659 && errorMelodyTone(360)==523);
  assert(errorMelodyTone(150)==0 && errorMelodyTone(720)==0);
  cal.stage=JoyCalStage::Sweep;
  assert(calibrationTone(cal,2048,2048,0,true)==0);
  assert(calibrationTone(cal,2090,2048,0,true)==0);
  assert(calibrationTone(cal,2304,2048,0,true)>1200); // Below motion threshold still audible.
  assert(calibrationTone(cal,1792,2048,0,true)<1200);
  assert(calibrationTone(cal,4095,2048,60,true)==0);
  assert(calibrationTone(cal,4095,2048,0,false)==0);
  cal.candidate.xAxis=1;
  assert(calibrationTone(cal,2048,4095,0,true)>1200);
  assert(calibrationTone(cal,0,4095,0,true)==400);
  cal.stage=JoyCalStage::Center;
  assert(calibrationTone(cal,0,0,0,true)==0);
  cal.finish(); assert(calibrationTone(cal,0,0,0,true)==0);
}
