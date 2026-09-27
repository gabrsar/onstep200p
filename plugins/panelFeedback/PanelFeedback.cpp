#include "PanelFeedback.h"
#include "../../Common.h"
#include "../../lib/tasks/OnTask.h"
#include "../../lib/wifi/WifiManager.h"
#include "../../telescope/mount/site/Site.h"
#include "../../telescope/mount/goto/Goto.h"
#include "../../telescope/mount/status/Status.h"
#include "../../telescope/Telescope.h"
#include <Preferences.h>
#include "../panelJoystick/PanelJoystick.h"
#include "CalibrationTone.h"
#include "BootMelody.h"
#include "ErrorMelody.h"
#include "FirmwareVersion.h"
#include <esp_ota_ops.h>
#include <esp_image_format.h>
#include <mbedtls/sha256.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace { void feedbackTick() { panelFeedback.loop(); } }
PanelFeedback panelFeedback;

const char *PanelFeedback::firmwareVersion() const { return onstep200pVersion; }
const char *PanelFeedback::firmwareHash() const {
  // Read once, including the appended image digest: same bytes as the .bin,
  // rather than the ELF hash or the ESP image-content-only hash.
  static char text[65]="UNAVAILABLE";
  static bool attempted=false;
  if (attempted) return text;
  attempted=true;
  const uint32_t hashStarted=millis();
  const esp_partition_t *partition=esp_ota_get_running_partition();
  if (!partition) return text;
  esp_partition_pos_t pos={partition->address,partition->size};
  esp_image_metadata_t image={};
  if (esp_image_verify(ESP_IMAGE_VERIFY_SILENT,&pos,&image)!=ESP_OK ||
      !image.image_len || image.image_len>partition->size) return text;
  mbedtls_sha256_context context;
  mbedtls_sha256_init(&context);
  bool ok=mbedtls_sha256_starts_ret(&context,0)==0;
  uint8_t block[1024],digest[32];
  for (uint32_t offset=0;ok && offset<image.image_len;) {
    const size_t size=image.image_len-offset<sizeof(block)?image.image_len-offset:sizeof(block);
    ok=esp_partition_read(partition,offset,block,size)==ESP_OK &&
      mbedtls_sha256_update_ret(&context,block,size)==0;
    offset+=size;
    if (!(offset%8192)) yield();
  }
  if (ok) ok=mbedtls_sha256_finish_ret(&context,digest)==0;
  mbedtls_sha256_free(&context);
  if (ok) {
    static const char hex[]="0123456789abcdef";
    for (int i=0;i<32;++i) { text[i*2]=hex[digest[i]>>4]; text[i*2+1]=hex[digest[i]&15]; }
    text[64]=0;
  }
  hashMs=millis()-hashStarted;
  return text;
}

void PanelFeedback::earlyMelodyTask(void *context) {
  PanelFeedback *self=static_cast<PanelFeedback *>(context);
  while (millis()-self->startupAt<bootMelodyDuration) {
    self->output(bootMelodyTone(millis()-self->startupAt));
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  self->output(0);
  self->earlyPlaying=false;
  vTaskDelete(nullptr);
}

void PanelFeedback::earlyBoot() {
  if (earlyStarted) return;
  earlyStarted=true; startupAt=millis();
  // Arduino initializes NVS before setup. Read mute before any startup tone.
  Preferences prefs;
  if (prefs.begin("onstep-panel",true)) {
    enabled=prefs.getBool("sound",true); prefs.end();
  }
  #if ONSTEP200P_SPEAKER != 0
    pinMode(ONSTEP200P_SPEAKER_PIN,OUTPUT);
    digitalWrite(ONSTEP200P_SPEAKER_PIN,LOW);
    if (enabled) {
      earlyPlaying=true;
      output(bootMelodyTone(0));
      if (xTaskCreate(earlyMelodyTask,"BootTune",2048,this,1,nullptr)!=pdPASS) {
        earlyPlaying=false; output(0);
      }
    }
  #endif
}

void PanelFeedback::output(unsigned hz) {
  if (hz==appliedHz) return;
  appliedHz=hz;
  #if ONSTEP200P_SPEAKER == 1
    digitalWrite(ONSTEP200P_SPEAKER_PIN,hz?HIGH:LOW);
  #elif ONSTEP200P_SPEAKER == 2
    if (hz) tone(ONSTEP200P_SPEAKER_PIN,hz); else noTone(ONSTEP200P_SPEAKER_PIN);
  #else
    (void)hz;
  #endif
}
const char *PanelFeedback::soundLabel() const {
  #if ONSTEP200P_SPEAKER == 0
    return "SOUND HW OFF";
  #else
    return enabled?"SOUND ON":"SOUND MUTED";
  #endif
}
void PanelFeedback::enable(bool value) {
  enabled=value;
  if (!enabled) { playing=false; output(0); }
  Preferences prefs;
  if (prefs.begin("onstep-panel",false)) {
    prefs.putBool("sound",enabled);
    prefs.end();
  }
}
void PanelFeedback::toggle() {
  enable(!enabled);
  if (enabled) play(BeepEvent::Ready);
}
void PanelFeedback::init() {
  if (!earlyStarted) earlyBoot();
  initialized=true;
  prevReady=site.isDateTimeReady();
  prevGoto=goTo.state!=GS_NONE;
  prevError=mountStatus.errorCode();
  #if OPERATIONAL_MODE == WIFI
    prevClients=WiFi.softAPgetStationNum();
    WiFi.setAutoReconnect(true);
  #endif
  startupErrorPending=prevError || initError.nv || initError.value || initError.driver || initError.gpio;
  tasks.add(20,0,true,7,feedbackTick,"Feedback");
}
void PanelFeedback::play(BeepEvent event) {
  if (!initialized || !enabled || ONSTEP200P_SPEAKER==0) return;
  const bool calCue=event==BeepEvent::CalStep || event==BeepEvent::CalDone;
  if (panelJoystick.calibrationStatus().active() && event!=BeepEvent::Error && !calCue) return;
  // Coalesce bursts: an error preempts, other events never queue stale tones.
  if (playing && event!=BeepEvent::Error && (!calCue || errorPattern)) return;
  calibrationCue=calCue;
  clientTune=event==BeepEvent::ClientConnected;
  clientLostTune=event==BeepEvent::ClientDisconnected;
  output(0);
  errorPattern=event==BeepEvent::Error;
  bootTune=event==BeepEvent::Boot || event==BeepEvent::Ready;
  readyTune=event==BeepEvent::Ready;
  networkTune=event==BeepEvent::WifiHome?NetworkRoute::Home:
    event==BeepEvent::WifiAccessPoint?NetworkRoute::AccessPoint:NetworkRoute::Off;
  bootTuneAt=millis();
  frequency=event==BeepEvent::Error?400:event==BeepEvent::Left?600:1600;
  pulses=beepPulses(event);
  if (event==BeepEvent::GotoStart) frequency=1000;
  if (event==BeepEvent::Ready) frequency=2200;
  index=0; phaseOn=false; playing=true; deadline=millis();
}
void PanelFeedback::loop() {
  if (earlyPlaying) return;
  if (startupErrorPending) { startupErrorPending=false; play(BeepEvent::Error); }
  const uint32_t now=millis();
  #if OPERATIONAL_MODE == WIFI
    // Retry even if the home network was absent at boot; keep AP available.
    if (wifiManager.settings.stationEnabled && now-reconnectAt>=30000) {
      reconnectAt=now;
      if (WiFi.status()!=WL_CONNECTED) WiFi.reconnect();
    }
  #endif
  if (panelJoystick.calibrationStatus().active() && playing && !errorPattern && !calibrationCue) {
    playing=false; output(0);
  }
  if (playing && (bootTune || errorPattern || clientTune || clientLostTune || networkTune!=NetworkRoute::Off)) {
    output(errorPattern?errorMelodyTone(now-bootTuneAt):
      clientTune?clientMelodyTone(now-bootTuneAt):
      clientLostTune?clientLostTone(now-bootTuneAt):
      networkTune!=NetworkRoute::Off?networkMelodyTone(networkTune,now-bootTuneAt):
      readyTune?readyMelodyTone(now-bootTuneAt):bootMelodyTone(now-bootTuneAt));
    const uint32_t duration=errorPattern?errorMelodyDuration:clientTune?clientMelodyDuration:clientLostTune?clientLostDuration:networkTune!=NetworkRoute::Off?networkMelodyDuration:
      readyTune?readyMelodyDuration:bootMelodyDuration;
    if (now-bootTuneAt>=duration) { playing=false; bootTune=false; }
  } else if (playing && (int32_t)(now-deadline)>=0) {
    if (!phaseOn) { output(frequency); phaseOn=true; deadline=now+100; }
    else {
      output(0); phaseOn=false; deadline=now+120;
      if (++index>=pulses) playing=false;
    }
  }
  if (!playing) {
    output(calibrationTone(panelJoystick.calibrationStatus(),
      panelJoystick.filteredInput(0),panelJoystick.filteredInput(1),now,
      enabled && !mountStatus.errorCode() && ONSTEP200P_SPEAKER!=0));
  }
  if (startupReadyPending && !playing && telescope.ready && now-startupAt>=bootMelodyDuration+400 &&
      !mountStatus.errorCode() && !initError.nv && !initError.value && !initError.driver && !initError.gpio &&
      !panelJoystick.calibrationStatus().active()) {
    startupReadyPending=false;
    play(BeepEvent::Ready);
  }
  if (now-sampledAt<200) return;
  sampledAt=now;
  const bool ready=site.isDateTimeReady(), going=goTo.state!=GS_NONE;
  const uint8_t error=mountStatus.errorCode();
  int clients=0;
  #if OPERATIONAL_MODE == WIFI
    clients=WiFi.softAPgetStationNum();
  #endif
  if (error && error!=prevError) play(BeepEvent::Error);
  else if (!error && going!=prevGoto) play(going?BeepEvent::GotoStart:BeepEvent::GotoEnd);
  else if (ready && !prevReady) play(BeepEvent::TimeReady);
  else if (clients!=prevClients) play(clients>prevClients?BeepEvent::Joined:BeepEvent::Left);
  prevReady=ready; prevGoto=going; prevError=error; prevClients=clients;
  if (clientNotice.disconnectedNotice(now)) clientLostPending=true;
  if ((clientPending || clientLostPending) && !playing && !startupReadyPending && !error && !panelJoystick.calibrationStatus().active()) {
    if (clientNotice.connected && clientPending) play(BeepEvent::ClientConnected);
    else if (!clientNotice.connected && clientLostPending) play(BeepEvent::ClientDisconnected);
    clientPending=clientLostPending=false;
  }
  #if OPERATIONAL_MODE == WIFI
    const bool ap=wifiManager.active && wifiManager.settings.accessPointEnabled;
    const bool available=!playing && !startupReadyPending && !error && !panelJoystick.calibrationStatus().active();
    const NetworkRoute notice=networkNotice.update(WiFi.status()==WL_CONNECTED,ap,
      wifiManager.settings.stationEnabled,now,available);
    if (notice==NetworkRoute::Home) play(BeepEvent::WifiHome);
    else if (notice==NetworkRoute::AccessPoint) play(BeepEvent::WifiAccessPoint);
  #endif
}
void PanelFeedback::observe(char channel,const char *cmd,const char *param,int error,const char *reply) {
  if (channel=='L') return;
  const char result=(error==CE_NONE || error==CE_1)?'1':'0';
  if (!strcmp(cmd,"SC")) { snprintf(clockDate,sizeof(clockDate),"%s",param); dateResult=result; clockChannel=channel; }
  if (!strcmp(cmd,"SL")) { snprintf(clockTime,sizeof(clockTime),"%s",param); timeResult=result; clockChannel=channel; }
  if (!strcmp(cmd,"SG")) { snprintf(clockZone,sizeof(clockZone),"%s",param); zoneResult=result; clockChannel=channel; }
  if (!strcmp(cmd,"CM") && !strcmp(reply,"N/A")) play(BeepEvent::Sync);
}
bool PanelFeedback::command(char *reply,char *cmd,char *param,bool *suppress,bool *numeric,CommandError *error) {
  if (!strcmp(cmd,"GX") && !strcmp(param,"PC")) {
    snprintf(reply,80,"TCP%d APP UNKNOWN PROTO LX200",clientNotice.connected);
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PW")) {
    #if OPERATIONAL_MODE == WIFI
      const bool home=WiFi.status()==WL_CONNECTED;
      const bool ap=wifiManager.active && wifiManager.settings.accessPointEnabled;
      snprintf(reply,80,"%s AP%d STA%d",home?"HOME":ap?"AP":"OFF",
        ap,wifiManager.settings.stationEnabled);
    #else
      snprintf(reply,80,"OFF AP0 STA0");
    #endif
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PQ")) {
    const auto ms=[](uint32_t value) { return unsigned(value>65535?65535:value); };
    snprintf(reply,80,"START%u H%u NV%u M%u TCP%u READY%u SHA%u",
      ms(startupAt),ms(bootTimes[0]),ms(bootTimes[1]),ms(bootTimes[2]),
      ms(bootTimes[3]),ms(bootTimes[4]),ms(hashMs));
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PA")) {
    snprintf(reply,80,"XF%d X%d YF%d Y%d",panelJoystick.firstInput(0),panelJoystick.settledInput(0),
      panelJoystick.firstInput(1),panelJoystick.settledInput(1));
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PM")) {
    const unsigned xmv=analogReadMilliVolts(ONSTEP200P_JOYSTICK_X_PIN);
    const unsigned xraw=analogReadRaw(ONSTEP200P_JOYSTICK_X_PIN);
    const unsigned ymv=analogReadMilliVolts(ONSTEP200P_JOYSTICK_Y_PIN);
    const unsigned yraw=analogReadRaw(ONSTEP200P_JOYSTICK_Y_PIN);
    snprintf(reply,80,"GPIO5 %umV RAW%u GPIO4 %umV RAW%u",xmv,xraw,ymv,yraw);
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PV")) {
    snprintf(reply,80,"%s %s",firmwareVersion(),firmwareHash());
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PK")) {
    const JoyProfile &p=panelJoystick.savedProfile();
    snprintf(reply,80,"P%d X%u,%u,%u Y%u,%u,%u AX1 SX%d SY%d DZ%u,%u CAL%u",
      panelJoystick.persistentCalibration(),unsigned(uint16_t(p.low[0])),unsigned(uint16_t(p.center[0])),unsigned(uint16_t(p.high[0])),
      unsigned(uint16_t(p.low[1])),unsigned(uint16_t(p.center[1])),unsigned(uint16_t(p.high[1])),
      p.xSign<0?-1:1,p.ySign<0?-1:1,unsigned(uint16_t(p.dead[0])),unsigned(uint16_t(p.dead[1])),
      unsigned(panelJoystick.calibrationStatus().stage));
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PJ")) {
    int x,y,cx,cy; bool down,ui; uint8_t clicks,cal;
    panelJoystick.diagnostics(x,y,cx,cy,down,clicks,ui,cal);
    snprintf(reply,80,"X%d Y%d CX%d CY%d B%d C%u %c %c",x,y,cx,cy,down,clicks,
      ui?'U':'M',panelJoystick.inputState());
    *numeric=false; *suppress=false; return true;
  }
  #if OPERATIONAL_MODE == WIFI
    if (!strcmp(cmd,"GX") && !strcmp(param,"PN")) {
      snprintf(reply,80,"STA %s AP %s",WiFi.status()==WL_CONNECTED?WiFi.localIP().toString().c_str():"OFF",
        WiFi.softAPIP().toString().c_str());
      *numeric=false; *suppress=false; return true;
    }
  #endif
  if (!strcmp(cmd,"GX") && !strcmp(param,"PT")) {
    snprintf(reply,80,"D%d T%d %c SC%c:%s SL%c:%s SG%c:%s",site.dateIsReady,site.timeIsReady,
      clockChannel,dateResult,clockDate,timeResult,clockTime,zoneResult,clockZone);
    *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"GX") && !strcmp(param,"PB")) {
    snprintf(reply,80,"%s",soundLabel()); *numeric=false; *suppress=false; return true;
  }
  if (!strcmp(cmd,"SX") && !strncmp(param,"PB,",3)) {
    if (!strcmp(param,"PB,0")) enable(false);
    else if (!strcmp(param,"PB,1")) { enable(true); play(BeepEvent::Ready); }
    else if (!strcmp(param,"PB,T")) play(BeepEvent::Boot);
    else if (!strcmp(param,"PB,E")) play(BeepEvent::Error);
    else *error=CE_PARAM_FORM;
    *numeric=true; return true;
  }
  return false;
}
void panelFeedbackObserveCommand(char channel,const char *cmd,const char *param,int error,const char *reply) {
  panelFeedback.observe(channel,cmd,param,error,reply);
}
void PanelFeedback::tcpConnection(bool connected) {
  if (clientNotice.update(connected,millis())) clientPending=true;
  if (!connected) clientPending=false;
  else clientLostPending=false;
}
void panelFeedbackTcpConnection(bool connected) { panelFeedback.tcpConnection(connected); }
