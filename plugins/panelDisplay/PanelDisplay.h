#pragma once

#include <Arduino.h>
#include "StatusEvents.h"

class PanelDisplay {
public:
  void init();
  void loop();
  void observeChannel(char channel);
  void boot(const char *stage);
  void control(bool enabled);
  void navigate(int8_t direction);

private:
  static constexpr uint8_t width = 128;
  static constexpr uint8_t height = 64;

  bool probe(uint8_t candidate);
  void beginDisplay();
  bool command(uint8_t value);
  bool writePacket(const uint8_t *data, size_t length);
  void clear();
  void drawChar(uint8_t x, uint8_t y, char value);
  void drawText(uint8_t x, uint8_t y, const char *text);
  void render();
  void flush();
  void pixel(int x, int y);
  void textLarge(uint8_t x, uint8_t y, const char *text);
  void row(uint8_t y, const char *label, const char *value);

  uint8_t address = 0;
  uint8_t framebuffer[width * height / 8] = {};
  uint8_t sentFrame[width * height / 8] = {};
  bool forceFrame = true;
  uint8_t requestHandle = 0;
  uint8_t query = 0;
  char values[11][40] = {};
  uint32_t startedAt = 0;
  uint32_t renderedAt = 0;
  uint16_t flushOffset = 1024;
  char sourceChannel = 0;
  uint32_t sourceAt = 0;
  bool uiControl = true;
  bool redraw = false;
  uint8_t selectedPage = 0;
  StatusEvents events;
  PanelEvent currentEvent = PanelEvent::None;
  uint32_t eventAt = 0;
  uint32_t lastProbeAt = 0;
  uint32_t lastQueryAt = 0;
  uint32_t sampleAt[11] = {};
};

extern PanelDisplay panelDisplay;
