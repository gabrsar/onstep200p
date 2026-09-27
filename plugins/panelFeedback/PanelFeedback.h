#pragma once
#include <Arduino.h>
#include <atomic>
#include "../../libApp/commands/ProcessCmds.h"

#include "BeepEvents.h"
#include "NetworkFeedback.h"
#include "ClientFeedback.h"
class PanelFeedback {
public:
  void earlyBoot();
  void bootStage(uint8_t stage) { if (stage<5) bootTimes[stage]=millis()-startupAt; }
  void init();
  void loop();
  bool command(char*,char*,char*,bool*,bool*,CommandError*);
  void observe(char channel,const char *cmd,const char *parameter,int error,const char *reply);
  void toggle();
  void click() { play(BeepEvent::Click); }
  void calibrationStep() { play(BeepEvent::CalStep); }
  void calibrationDone() { play(BeepEvent::CalDone); }
  void tcpConnection(bool connected);
  bool tcpConnected() const { return clientNotice.connected; }
  const char *soundLabel() const;
  const char *firmwareVersion() const;
  const char *firmwareHash() const;
private:
  static void earlyMelodyTask(void *context);
  void enable(bool value);
  void play(BeepEvent event);
  void output(unsigned frequency);
  bool initialized=false, enabled=true, playing=false, phaseOn=false;
  uint8_t pulses=0, index=0;
  unsigned frequency=0;
  unsigned appliedHz=0;
  bool errorPattern=false;
  bool startupReadyPending=true;
  uint32_t startupAt=0;
  bool earlyStarted=false;
  std::atomic<bool> earlyPlaying{false};
  bool startupErrorPending=false;
  uint32_t bootTimes[5]={};
  mutable uint32_t hashMs=0;
  bool bootTune=false;
  bool readyTune=false;
  NetworkRoute networkTune=NetworkRoute::Off;
  bool calibrationCue=false;
  bool clientTune=false,clientPending=false;
  bool clientLostTune=false,clientLostPending=false;
  ClientNotice clientNotice;
  NetworkNotice networkNotice;
  uint32_t bootTuneAt=0;
  uint32_t deadline=0, sampledAt=0;
  uint32_t reconnectAt=0;
  bool prevReady=false,prevGoto=false;
  uint8_t prevError=0;
  int prevClients=0;
  char clockDate[14]="--",clockTime[16]="--",clockZone[10]="--";
  char dateResult='?',timeResult='?',zoneResult='?',clockChannel='?';
};
extern PanelFeedback panelFeedback;
