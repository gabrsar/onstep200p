#include "PanelDisplay.h"
#include "JoyArrows.h"

#include "../../Common.h"
#include "../../lib/tasks/OnTask.h"
#include "../../libApp/commands/CommandBroker.h"
#include "../../lib/wifi/WifiManager.h"
#include "../../telescope/Telescope.h"
#include "../panelJoystick/PanelJoystick.h"
#include "../panelFeedback/PanelFeedback.h"
#include <math.h>

namespace {

const char *queries[] = {":GZ#", ":GA#", ":GU#", ":GL#", ":GC#",
                        ":Gt#", ":Gg#", ":Gv#", ":GG#", ":A?#", ":GX89#"};
void degreesText(char *out, size_t size, const char *input) {
  float degrees=0, seconds=0;
  unsigned minutes=0;
  if (sscanf(input,"%f*%u:%f",&degrees,&minutes,&seconds) < 2) {
    snprintf(out,size,"--"); return;
  }
  float value=fabsf(degrees)+minutes/60.0f+seconds/3600.0f;
  if (input[0]=='-') value=-value;
  snprintf(out,size,"%.2f",value);
}
// Preserve credential case on screen (the original font only had capitals).
const uint8_t lowercase[][5] = {
 {0x20,0x54,0x54,0x54,0x78},{0x7f,0x48,0x44,0x44,0x38},
 {0x38,0x44,0x44,0x44,0x20},{0x38,0x44,0x44,0x48,0x7f},
 {0x38,0x54,0x54,0x54,0x18},{0x08,0x7e,0x09,0x01,0x02},
 {0x0c,0x52,0x52,0x52,0x3e},{0x7f,0x08,0x04,0x04,0x78},
 {0,0x44,0x7d,0x40,0},{0x20,0x40,0x44,0x3d,0},
 {0x7f,0x10,0x28,0x44,0},{0,0x41,0x7f,0x40,0},
 {0x7c,0x04,0x18,0x04,0x78},{0x7c,0x08,0x04,0x04,0x78},
 {0x38,0x44,0x44,0x44,0x38},{0x7c,0x14,0x14,0x14,0x08},
 {0x08,0x14,0x14,0x18,0x7c},{0x7c,0x08,0x04,0x04,0x08},
 {0x48,0x54,0x54,0x54,0x20},{0x04,0x3f,0x44,0x40,0x20},
 {0x3c,0x40,0x40,0x20,0x7c},{0x1c,0x20,0x40,0x20,0x1c},
 {0x3c,0x40,0x30,0x40,0x3c},{0x44,0x28,0x10,0x28,0x44},
 {0x0c,0x50,0x50,0x50,0x3c},{0x44,0x64,0x54,0x4c,0x44}
};

// Characters needed by the status screen: space, punctuation, digits and A-Z.
const char glyphCharacters[] = " -+.0123456789:?ABCDEFGHIJKLMNOPQRSTUVWXYZ<>^!";
const uint8_t glyphs[][5] = {
  {0x00, 0x00, 0x00, 0x00, 0x00}, // space
  {0x08, 0x08, 0x08, 0x08, 0x08}, // -
  {0x08, 0x08, 0x3e, 0x08, 0x08}, // +
  {0x00, 0x60, 0x60, 0x00, 0x00}, // .
  {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
  {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
  {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
  {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
  {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
  {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
  {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
  {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
  {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
  {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
  {0x00, 0x36, 0x36, 0x00, 0x00}, // :
  {0x02, 0x01, 0x51, 0x09, 0x06}, // ?
  {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
  {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
  {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
  {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
  {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
  {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
  {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
  {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
  {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
  {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
  {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
  {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
  {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
  {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
  {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
  {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
  {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
  {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
  {0x46, 0x49, 0x49, 0x49, 0x31}, // S
  {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
  {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
  {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
  {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
  {0x63, 0x14, 0x08, 0x14, 0x63}, // X
  {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
  {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
  {0x08, 0x14, 0x22, 0x41, 0x00}, // <
  {0x41, 0x22, 0x14, 0x08, 0x00}, // >
  {0x04, 0x02, 0x01, 0x02, 0x04}, // ^
  {0x00, 0x00, 0x5f, 0x00, 0x00}, // !
};

void panelDisplayWrapper() {
  panelDisplay.loop();
}

} // namespace

bool PanelDisplay::probe(uint8_t candidate) {
  HAL_WIRE.beginTransmission(candidate);
  return HAL_WIRE.endTransmission() == 0;
}

bool PanelDisplay::writePacket(const uint8_t *data, size_t length) {
  if (!address) return false;
  HAL_WIRE.beginTransmission(address);
  const bool complete=HAL_WIRE.write(data,length)==length;
  if (HAL_WIRE.endTransmission()!=0 || !complete) {
    address=0;
    lastProbeAt=millis();
    forceFrame=true;
    return false;
  }
  return true;
}

bool PanelDisplay::command(uint8_t value) {
  const uint8_t packet[]={0x00,value};
  return writePacket(packet,sizeof(packet));
}

void PanelDisplay::beginDisplay() {
  const uint8_t initSequence[] = {
    0xae, 0xd5, 0x80, 0xa8, 0x3f, 0xd3, 0x00, 0x40,
    0x8d, 0x14, 0x20, 0x00, 0xa1, 0xc8, 0xda, 0x12,
    0x81, 0x7f, 0xd9, 0xf1, 0xdb, 0x40, 0xa4, 0xa6, 0xaf
  };
  for (uint8_t value : initSequence) if (!command(value)) break;
  forceFrame=true;
}

void PanelDisplay::clear() {
  memset(framebuffer, 0, sizeof(framebuffer));
}

void PanelDisplay::drawChar(uint8_t x, uint8_t y, char value) {
  if (value == '*') value = '.';
  if (value == '/') value = '-';
  const char *match = strchr(glyphCharacters, value);
  if (x + 5 >= width || y + 7 > height) return;
  const uint8_t *glyph = value >= 'a' && value <= 'z' ? lowercase[value-'a'] :
    (match ? glyphs[match-glyphCharacters] : glyphs[strchr(glyphCharacters,'?')-glyphCharacters]);
  for (uint8_t column = 0; column < 5; column++) {
    for (uint8_t row = 0; row < 7; row++) {
      if (glyph[column] & (1U << row)) {
        const uint16_t index = x + column + ((y + row) / 8) * width;
        framebuffer[index] |= 1U << ((y + row) % 8);
      }
    }
  }
}

void PanelDisplay::drawText(uint8_t x, uint8_t y, const char *text) {
  while (*text != '\0' && x + 5 < width) {
    drawChar(x, y, *text++);
    x += 6;
  }
}

void PanelDisplay::flush() {
  while (flushOffset<sizeof(framebuffer)) {
    if (!forceFrame && memcmp(framebuffer+flushOffset,sentFrame+flushOffset,32)==0) {
      flushOffset+=32;
      continue;
    }
    const uint8_t column=flushOffset%width, page=flushOffset/width;
    const uint8_t window[]={0x00,0x21,column,(uint8_t)(column+31),0x22,page,page};
    if (!writePacket(window,sizeof(window))) return;
    uint8_t packet[33];
    packet[0]=0x40;
    memcpy(packet+1,framebuffer+flushOffset,32);
    if (!writePacket(packet,sizeof(packet))) return;
    memcpy(sentFrame+flushOffset,framebuffer+flushOffset,32);
    flushOffset+=32;
    break;
  }
  if (flushOffset>=sizeof(framebuffer)) forceFrame=false;
}

void PanelDisplay::pixel(int x, int y) {
  if (x >= 0 && x < width && y >= 0 && y < height)
    framebuffer[x + (y / 8) * width] |= 1U << (y % 8);
}

void PanelDisplay::textLarge(uint8_t x, uint8_t y, const char *text) {
  while (*text && x + 10 < width) {
    char c = *text++;
    if (c == '*') c = '.';
    const char *p = strchr(glyphCharacters, c);
    if (p) for (int col = 0; col < 5; ++col) for (int r = 0; r < 7; ++r)
      if (glyphs[p-glyphCharacters][col] & (1U << r))
        for (int dx = 0; dx < 2; ++dx) for (int dy = 0; dy < 2; ++dy)
          pixel(x + col*2 + dx, y + r*2 + dy);
    x += 12;
  }
}

void PanelDisplay::row(uint8_t y, const char *label, const char *value) {
  drawText(0, y, label);
  drawText(30, y, value[0] ? value : "--");
}

void PanelDisplay::render() {
  clear();
  const uint32_t age = millis() - startedAt;
  if (age < 1200) {
    // Original orbital mark, drawn procedurally without bitmap dependencies.
    for (int i = 0; i < 180; ++i) {
      float a = i * 0.0349066f;
      pixel(64 + 29*cosf(a), 22 + 10*sinf(a) + 6*cosf(a));
    }
    for (int d = -7; d <= 7; ++d) { pixel(64+d,22); pixel(64,22+d); }
    drawText(34, 40, "ONSTEP200P");
    char identity[24];
    snprintf(identity,sizeof(identity),"VERSION %s",panelFeedback.firmwareVersion());
    drawText(25,48,identity);
    snprintf(identity,sizeof(identity),"BIN %.12s",panelFeedback.firmwareHash());
    drawText(16,56,identity);
  } else {
    const uint8_t page = selectedPage;
    const char *titles[] = {"MOUNT", "OBSERVATORY", "NETWORK", "ACCESS", "SYSTEM", "INPUT"};
    const bool calScreen=panelJoystick.showCalibration();
    drawText(2, 0, calScreen?"JOY CAL":titles[page]);
    drawText(72,0,uiControl?"U":"M");
    // Live controls stay visible on every page, even while calibration blocks input.
    for (int dx=0;dx<5;++dx) for (int dy=0;dy<5;++dy)
      if (dx==0 || dx==4 || dy==0 || dy==4 || panelJoystick.clickLit()) pixel(83+dx,1+dy);
    uint16_t arrow[9]; joyArrow(panelJoystick.horizontal(),panelJoystick.vertical(),arrow,
      panelJoystick.persistentCalibration()?0:450);
    for (int row=0;row<9;++row) for (int col=0;col<9;++col)
      if (arrow[row] & (1u<<col)) pixel(94+col,row);
    const char state[]={panelJoystick.inputState(),0}; drawText(107,0,state);
    const char pageNumber[]={char('1'+page),0}; drawText(120,0,pageNumber);
    for (int x=0; x<128; ++x) pixel(x,10);
    if (calScreen) {
      const JoyCalibration &cal=panelJoystick.calibrationStatus();
      const char *instruction="SAVED";
      char poseLabel[24];
      switch (cal.stage) {
        case JoyCalStage::Release: instruction="RELEASE BUTTON"; break;
        case JoyCalStage::Reference: instruction="CENTER TO START"; break;
        case JoyCalStage::ConfirmCenter: instruction="CLICK THEN CENTER"; break;
        case JoyCalStage::Center:
          snprintf(poseLabel,sizeof(poseLabel),"FINGER CENTER %u/3",cal.centerSeconds(millis())); instruction=poseLabel; break;
        case JoyCalStage::Sweep:
          snprintf(poseLabel,sizeof(poseLabel),"FULL CIRCLES %u/3",unsigned(cal.laps)); instruction=poseLabel; break;
        case JoyCalStage::Save: instruction="SAVING"; break;
        default: instruction=cal.notice; break;
      }
      drawText(0,14,instruction);
      drawText(12,24,"MIN"); drawText(42,24,"CUR"); drawText(72,24,"MAX");
      char limits[24];
      for (int axis=0;axis<2;++axis) {
        char lo[8]="--",hi[8]="--";
        if (cal.candidate.low[axis]>=0) snprintf(lo,sizeof(lo),"%d",cal.candidate.low[axis]);
        if (cal.candidate.high[axis]>=0) snprintf(hi,sizeof(hi),"%d",cal.candidate.high[axis]);
        snprintf(limits,sizeof(limits),"%c %4s %4d %4s",axis?'Y':'X',lo,panelJoystick.filteredInput(axis),hi);
        drawText(0,34+axis*10,limits);
      }
      drawText(0,54,cal.notice[0]?cal.notice:cal.stage==JoyCalStage::Sweep?"NO CLICKS - 3 TURNS":
        cal.stage==JoyCalStage::Center?"HOLD LIGHTLY - 3 SEC":"MOTION DISABLED");
    } else if (page == 0) {
      const char *s = values[2];
      const char *mode = "--";
      if (*s) {
        if (strlen(s) >= 3 && s[strlen(s)-1] != '0') mode="MOUNT ERROR";
        else if (strchr(s,'F')) mode="PARK ERROR";
        else if (strchr(s,'P')) mode="PARKED";
        else if (strchr(s,'I')) mode="PARKING";
        else if (strchr(s,'h')) mode="HOMING";
        else if (!strchr(s,'N')) mode="GOTO";
        else if (values[9][1] && values[9][1]!='0' && values[9][2]!='0') mode="ALIGNMENT";
        else if (strchr(s,'g') || strchr(s,'G')) mode="GUIDING";
        else if (!strchr(s,'n')) mode="TRACKING";
        else mode="IDLE";
      }
      drawText(0,14,mode);
      drawText(0,30,"AZ"); drawText(0,48,"ALT");
      char compact[9];
      degreesText(compact,sizeof(compact),values[0]);
      textLarge(30,27,compact);
      degreesText(compact,sizeof(compact),values[1]);
      textLarge(30,45,compact);
    } else if (page == 1) {
      row(14,"TIME",values[10][0]=='0'?values[3]:"NOT SYNCED");
      row(24,"DATE",values[4]); row(34,"LAT",values[5]);
      char lng[40]; snprintf(lng,sizeof(lng),"%s",values[6]);
      // LX200 longitude is positive west. Display explicitly to avoid ambiguity.
      if (lng[0]=='+' || lng[0]=='-') lng[0] = lng[0]=='+'?'W':'E';
      row(44,"LNG",lng);
      char elevation[24]; snprintf(elevation,sizeof(elevation),"%.16s m",values[7][0]?values[7]:"--");
      row(54,"ELEV",elevation);
    } else if (page == 2 || page == 3) {
      #if OPERATIONAL_MODE == WIFI
        if (page == 2) {
          const bool connected=WiFi.status()==WL_CONNECTED;
          drawText(0,14,connected?"HOME WIFI CONNECTED":wifiManager.active?"AP FALLBACK":"WIFI OFF");
          drawText(0,24,(connected?WiFi.localIP():WiFi.softAPIP()).toString().c_str());
          drawText(0,34,"TCP 9999");
          drawText(0,44,panelFeedback.tcpConnected()?"APP LX200 CONNECTED":"APP --");
          const String ssid=connected?WiFi.SSID():String(wifiManager.settings.ap.ssid);
          const unsigned length=ssid.length();
          const unsigned offset=length>21?(millis()/500)%(length+6):0;
          const String scrolling=ssid+"      "+ssid;
          drawText(0,54,scrolling.substring(offset,offset+21).c_str());
        } else {
          drawText(0,14,"SSID"); drawText(0,24,wifiManager.settings.ap.ssid);
          drawText(0,36,"PASSWORD"); drawText(0,46,wifiManager.settings.ap.pwd);
        }
      #else
        drawText(0,24,"WIFI DISABLED");
      #endif
    } else if (page == 4) {
      const bool recent = sourceChannel && millis()-sourceAt < 10000;
      const char *source = sourceChannel=='A'?"USB":
        (sourceChannel=='I' || (sourceChannel>='1' && sourceChannel<='3'))?"WIFI TCP":
        sourceChannel?"SERIAL":"--";
      row(14,recent?"LIVE":"LAST",source);
      drawText(0,24,"VCC NO SENSOR");
      drawText(0,34,panelFeedback.soundLabel());
      row(44,"L-UT",values[8]); // Offset added to local time to obtain UTC (LX200).
      const char *error = initError.nv ? "ERR NV STORAGE" :
        initError.value ? "ERR CONFIG VALUE" : initError.driver ? "ERR DRIVER" :
        initError.weather ? "ERR WEATHER" : initError.tls ? "ERR TIME SOURCE" :
        initError.gpio ? "ERR GPIO" : "INIT OK";
      drawText(0,54,error);
    } else {
      int x,y,cx,cy;
      bool button,ui;
      uint8_t presses,calibration;
      panelJoystick.diagnostics(x,y,cx,cy,button,presses,ui,calibration);
      char line[30];
      snprintf(line,sizeof(line),"X %d C %d",x,cx); drawText(0,14,line);
      snprintf(line,sizeof(line),"Y %d C %d",y,cy); drawText(0,24,line);
      snprintf(line,sizeof(line),"SW %s  CLICKS %u",button?"DOWN":"UP",presses);
      drawText(0,34,line);
      snprintf(line,sizeof(line),"MODE %s",ui?"UI":"MOUNT"); drawText(0,44,line);
      drawText(0,54,panelJoystick.stopping()?"STOP RETRY":
        panelJoystick.calibrated()?"CENTER OK":"CENTER STICK TO CAL");
    }
  }
  if (age>=3000 && !panelJoystick.showCalibration() && currentEvent != PanelEvent::None && millis()-eventAt < 2500) {
    // Status toast replaces only the last 8-pixel text row.
    for (int x=0;x<width;++x) framebuffer[6*width+x]&=0x3f;
    memset(framebuffer + 7*width, 0, width);
    for (int x=0; x<width; ++x) pixel(x,55);
    drawText(1,57,panelEventText(currentEvent));
  }
  flushOffset = 0;
  renderedAt = millis();
}

void PanelDisplay::init() {
  VLF("MSG: Plugins, starting: PanelDisplay");

  HAL_WIRE.setTimeOut(20);
  lastProbeAt=millis();
  if (address) { /* Already initialized by the early boot hook. */ }
  else if (probe(0x3c)) address = 0x3c;
  else if (probe(0x3d)) address = 0x3d;
  else {
    DLF("WRN: PanelDisplay, no SSD1306 at 0x3C or 0x3D");
  }

  HAL_WIRE.setTimeOut(20);
  HAL_WIRE.setClock(400000);
  startedAt = millis();
  beginDisplay();
  render();
  tasks.add(10, 0, true, 7, panelDisplayWrapper, "PanelDisp");
}

void PanelDisplay::loop() {
  if (!address && millis()-lastProbeAt>=5000) {
    lastProbeAt=millis();
    if (probe(0x3c)) address=0x3c;
    else if (probe(0x3d)) address=0x3d;
    if (address) { beginDisplay(); flushOffset=sizeof(framebuffer); redraw=true; }
  }
  for (uint8_t i=0;i<11;++i)
    if (values[i][0] && millis()-sampleAt[i]>5000) values[i][0]=0;
  // Four bounded 32-byte transfers: about 3 ms of bus time at 400 kHz.
  for (uint8_t i=0; i<4 && address; ++i) flush();
  if (address && flushOffset >= sizeof(framebuffer) && (redraw || millis()-renderedAt >= 100)) {
    redraw = false;
    render();
  }

  if (requestHandle == 0) {
    if (millis()-lastQueryAt<75) return;
    requestHandle = commandBroker.request(queries[query], 250);
    if (requestHandle) lastQueryAt=millis();
    return;
  }

  char reply[40] = "";
  const CommandBrokerStatus status = commandBroker.result(requestHandle, reply, sizeof(reply));
  if (status != CB_DONE && status != CB_TIMEOUT) return;

  char *target = values[query];
  if (status == CB_DONE && reply[0] != '\0') {
    reply[strcspn(reply,"#\r\n")] = '\0';
    strncpy(target, reply, 39);
    target[39] = '\0';
    sampleAt[query]=millis();
  } else {
    target[0] = '\0';
  }

  query = (query + 1) % 11;
  if (query == 3) {
    int clients = 0;
    #if OPERATIONAL_MODE == WIFI
      clients = WiFi.softAPgetStationNum();
    #endif
    const PanelEvent event = events.observe(target, clients);
    if (event != PanelEvent::None) {
      currentEvent = event;
      eventAt = millis();
      redraw = true;
    }
  }
}

PanelDisplay panelDisplay;

void PanelDisplay::observeChannel(char channel) {
  if (channel == 'L') return; // Internal OLED/joystick queries are not external clients.
  sourceChannel = channel;
  sourceAt = millis();
}

void panelDisplayObserveChannel(char channel) {
  panelDisplay.observeChannel(channel);
}

void PanelDisplay::control(bool enabled) {
  uiControl = enabled;
  currentEvent=PanelEvent::None;
  redraw = true;
}

void PanelDisplay::navigate(int8_t direction) {
  selectedPage = (selectedPage + (direction > 0 ? 1 : 5)) % 6;
  redraw = true;
}

void PanelDisplay::boot(const char *stage) {
  if (!address) {
    HAL_WIRE.setTimeOut(20);
    HAL_WIRE.setClock(400000);
    if (probe(0x3c)) address=0x3c;
    else if (probe(0x3d)) address=0x3d;
    else return;
    beginDisplay();
  }
  clear();
  drawText(16,8,"ONSTEP200P");
  char identity[24];
  snprintf(identity,sizeof(identity),"VERSION %s",panelFeedback.firmwareVersion());
  drawText(0,20,identity);
  snprintf(identity,sizeof(identity),"BIN %.12s",panelFeedback.firmwareHash());
  drawText(0,30,identity);
  drawText(0,48,stage);
  flushOffset=0;
  // Setup only: scheduler/broker are not required for boot diagnostics.
  while (address && flushOffset < sizeof(framebuffer)) flush();
}
