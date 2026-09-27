#pragma once

#include <Arduino.h>
#include "ClickGesture.h"
#include "PageGesture.h"
#include "ManualMotion.h"
#include "CenterCalibration.h"
#include "JoyIndicator.h"
#include "JoyCalibration.h"

class PanelJoystick {
public:
  void init();
  void loop();
  void diagnostics(int &x, int &y, int &centerX, int &centerY,
                   bool &button, uint8_t &presses, bool &ui, uint8_t &calibration) const;
  bool calibrated() const { return calibrationReady && !wizard.active(); }
  bool stopping() const { return motion.pendingStop(); }
  bool clickLit() const { return clickIndicator.lit(millis()); }
  char inputState() const { return stopping()?'!':!calibrated()?'?':neutralRequired?'-':'+'; }
  int horizontal() const { return normalizedX; }
  int vertical() const { return normalizedY; }
  int filteredInput(uint8_t axis) const { return filteredInputs[axis]; }
  int firstInput(uint8_t axis) const { return firstInputs[axis]; }
  int settledInput(uint8_t axis) const { return axis?lastRawY:lastRawX; }
  const JoyCalibration &calibrationStatus() const { return wizard; }
  bool showCalibration() const { return wizard.active() || (finishedCal && millis()-finishedCal<2500); }
  const JoyProfile &savedProfile() const { return profile; }
  bool persistentCalibration() const { return persisted; }

private:
  int8_t directionFor(int value, int center, int8_t current) const;
  void stopAll();

  static constexpr int enterDeadZone = 700;
  static constexpr int exitDeadZone = 450;
  static constexpr unsigned long refreshMs = 5000;

  CenterCalibration calibration;
  JoyProfile profile;
  JoyCalibration wizard;
  Median5 filterX,filterY;
  int normalizedX=0,normalizedY=0;
  int filteredInputs[2]={2048,2048};
  int firstInputs[2]={2048,2048};
  bool stableButton=false,debounceRaw=false;
  uint32_t buttonChangedAt=0,finishedCal=0;
  bool persisted=false;
  bool calibrationReady = false;
  ManualMotion motion;
  int centerX = 2048;
  int centerY = 2048;
  bool previousButton = false;
  unsigned long buttonHeldAt = 0;
  bool holdHandled = false;
  ClickGesture gesture;
  bool uiControl = true;
  bool neutralRequired = true;
  PageGesture pageGesture;
  unsigned long lastRefresh = 0;
  int lastRawX = 0;
  int lastRawY = 0;
  bool lastButton = false;
  ClickIndicator clickIndicator;
};

extern PanelJoystick panelJoystick;
