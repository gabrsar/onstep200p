#include "PanelJoystick.h"

#include "../../Common.h"
#include "../../lib/tasks/OnTask.h"
#include "../../libApp/commands/CommandBroker.h"
#include "../panelFeedback/PanelFeedback.h"
#include <Preferences.h>
#if ONSTEP200P_DISPLAY == ON
#include "../panelDisplay/PanelDisplay.h"
#endif

namespace {

void panelJoystickWrapper() {
  panelJoystick.loop();
}

} // namespace

int8_t PanelJoystick::directionFor(int value, int center, int8_t current) const {
  const int delta = value - center;
  const int enter=persisted?1:enterDeadZone, exit=persisted?0:exitDeadZone;
  if (delta >= enter) return 1;
  if (delta <= -enter) return -1;
  if (current != 0 && delta*current > exit) return current;
  return 0;
}

void PanelJoystick::stopAll() {
  motion.stop(commandBroker);
  neutralRequired = true;
  lastRefresh = millis();
}

void PanelJoystick::init() {
  VLF("MSG: Plugins, starting: PanelJoystick");
  pinMode(ONSTEP200P_JOYSTICK_SW_PIN, INPUT_PULLUP);
  pinMode(ONSTEP200P_JOYSTICK_X_PIN, INPUT);
  pinMode(ONSTEP200P_JOYSTICK_Y_PIN, INPUT);
  analogSetPinAttenuation(ONSTEP200P_JOYSTICK_X_PIN,ADC_11db);
  analogSetPinAttenuation(ONSTEP200P_JOYSTICK_Y_PIN,ADC_11db);
  Preferences prefs;
  if (prefs.begin("onstep-joy",true)) {
    JoyProfile stored;
    if (prefs.getBytesLength("cal-v4")==sizeof(stored) &&
        prefs.getBytes("cal-v4",&stored,sizeof(stored))==sizeof(stored) && stored.valid()) {
      profile=stored; centerX=profile.center[0]; centerY=profile.center[1];
      calibrationReady=true;
      persisted=true;
    }
    prefs.end();
  }
  #if ONSTEP200P_DISPLAY != ON
    uiControl = false;
  #endif
  tasks.add(10, 0, true, 7, panelJoystickWrapper, "PanelJoy");
}

void PanelJoystick::loop() {
  firstInputs[0]=joyAdc12(analogRead(ONSTEP200P_JOYSTICK_X_PIN),ANALOG_READ_RANGE);
  delayMicroseconds(10);
  const int rawX = joyAdc12(analogRead(ONSTEP200P_JOYSTICK_X_PIN),ANALOG_READ_RANGE);
  firstInputs[1]=joyAdc12(analogRead(ONSTEP200P_JOYSTICK_Y_PIN),ANALOG_READ_RANGE);
  delayMicroseconds(10);
  const int rawY = joyAdc12(analogRead(ONSTEP200P_JOYSTICK_Y_PIN),ANALOG_READ_RANGE);
  lastRawX = rawX;
  lastRawY = rawY;
  lastButton = digitalRead(ONSTEP200P_JOYSTICK_SW_PIN) == LOW;
  clickIndicator.update(lastButton,millis());
  const uint32_t now=millis();
  const int filteredX=filterX.update(rawX),filteredY=filterY.update(rawY);
  filteredInputs[0]=filteredX; filteredInputs[1]=filteredY;
  profile.normalize(filteredX,filteredY,normalizedX,normalizedY);

  // The stop button remains available during calibration and queue recovery.
  const bool button = lastButton;
  if (button && !previousButton) stopAll();
  previousButton = button;
  if (button!=debounceRaw) { debounceRaw=button; buttonChangedAt=now; }
  bool clicked=false,released=false;
  if (button!=stableButton && now-buttonChangedAt>=30) {
    stableButton=button; clicked=button; released=!button;
    if (clicked) { buttonHeldAt=now; holdHandled=false; }
  }
  if (motion.pendingStop()) { motion.serviceStop(commandBroker); return; }

  if (!wizard.active()) {
    if (stableButton && !holdHandled && now-buttonHeldAt>=5000) {
      holdHandled=true; stopAll(); wizard.begin(now,profile.xSign,profile.ySign); finishedCal=0;
      panelFeedback.calibrationStep();
      gesture=ClickGesture(); pageGesture.reset(); uiControl=true;
      #if ONSTEP200P_DISPLAY == ON
        panelDisplay.control(true);
      #endif
    } else if (released && !holdHandled && now-buttonHeldAt>=1500) {
      // Short long-press toggles sound ON RELEASE, never on the way to 5 s.
      holdHandled=true; panelFeedback.toggle(); gesture=ClickGesture();
      neutralRequired=true; return;
    }
  }
  if (wizard.active()) {
    const JoyCalStage previousStage=wizard.stage;
    if (wizard.update(filteredX,filteredY,stableButton,clicked,now,rawX,rawY)) {
      Preferences prefs;
      bool saved=false;
      if (prefs.begin("onstep-joy",false)) {
        saved=prefs.putBytes("cal-v4",&wizard.candidate,sizeof(JoyProfile))==sizeof(JoyProfile);
        prefs.end();
      }
      if (saved) {
        profile=wizard.candidate; centerX=profile.center[0]; centerY=profile.center[1];
        calibrationReady=true; wizard.finish(); wizard.notice="SAVED";
        persisted=true;
        panelFeedback.calibrationDone();
      } else wizard.saveFailed(now);
    }
    if (previousStage==JoyCalStage::Sweep && wizard.stage==JoyCalStage::ConfirmCenter)
      panelFeedback.calibrationStep();
    if (!wizard.active()) finishedCal=now;
    gesture=ClickGesture(); pageGesture.reset(); neutralRequired=true;
    return;
  }

  if (!calibrationReady) {
    if (calibration.add(filteredX,filteredY)) {
      if (!calibration.valid()) { calibration=CenterCalibration(); return; }
      centerX = calibration.x();
      centerY = calibration.y();
      profile.center[0]=centerX; profile.center[1]=centerY;
      calibrationReady = true;
      VF("MSG: PanelJoystick, center X="); V(centerX);
      VF(" Y="); VL(centerY);
    }
    return;
  }

  if (gesture.update(button, millis())) {
    stopAll();
    neutralRequired = true;
    pageGesture.reset();
    #if ONSTEP200P_DISPLAY == ON
      uiControl = !uiControl;
      panelDisplay.control(uiControl);
    #endif
  }
  if (gesture.takeSingle()) {
    stopAll(); pageGesture.reset();
    #if ONSTEP200P_DISPLAY == ON
      panelDisplay.navigate(1);
      panelFeedback.click();
    #endif
    return;
  }
  if (gesture.blocked()) { neutralRequired = true; pageGesture.reset(); return; }

  const int8_t x = directionFor(normalizedX, 0, motion.direction(0));
  const int8_t y = directionFor(normalizedY, 0, motion.direction(1));
  if (neutralRequired) {
    const int neutral=persisted?0:exitDeadZone;
    if (abs(normalizedX) <= neutral && abs(normalizedY) <= neutral)
      neutralRequired = false;
    return;
  }
  if (uiControl) {
    #if ONSTEP200P_DISPLAY == ON
      const int8_t direction = pageGesture.update(normalizedX,normalizedY,millis(),persisted?1:700,persisted?0:450);
      if (direction) panelDisplay.navigate(direction);
    #endif
    return;
  }

  const bool changed=x!=motion.direction(0) || y!=motion.direction(1);
  if (!motion.update(commandBroker,x,y)) { neutralRequired=true; return; }
  if (changed) lastRefresh=millis();

  if ((motion.direction(0) || motion.direction(1)) && millis()-lastRefresh >= refreshMs) {
    if (!motion.refresh(commandBroker)) neutralRequired=true;
    lastRefresh = millis();
  }
}

PanelJoystick panelJoystick;

void PanelJoystick::diagnostics(int &x, int &y, int &cx, int &cy,
                                bool &button, uint8_t &presses, bool &ui,
                                uint8_t &calibration) const {
  x = lastRawX;
  y = lastRawY;
  cx = centerX;
  cy = centerY;
  button = lastButton;
  presses = gesture.presses();
  ui = uiControl;
  calibration = this->calibration.remaining();
}
