#pragma once
#include <stdint.h>

enum class NetworkRoute : uint8_t { Off, AccessPoint, Home };

// Only the latest stable state is offered when audio is available. No FIFO
// of obsolete Wi-Fi transitions, and no delay in the controller startup.
class NetworkNotice {
public:
  NetworkRoute update(bool home,bool ap,bool tryingHome,uint32_t now,bool canAnnounce) {
    const NetworkRoute next=home?NetworkRoute::Home:ap?NetworkRoute::AccessPoint:NetworkRoute::Off;
    if (!seen || next!=candidate) { seen=true; candidate=next; changedAt=now; }
    const uint32_t hold=next==NetworkRoute::AccessPoint && tryingHome?8000:1000;
    if (now-changedAt<hold || !canAnnounce || candidate==announced) return NetworkRoute::Off;
    announced=candidate;
    return candidate;
  }
private:
  bool seen=false;
  NetworkRoute candidate=NetworkRoute::Off,announced=NetworkRoute::Off;
  uint32_t changedAt=0;
};

static constexpr uint32_t networkMelodyDuration=480;
inline unsigned networkMelodyTone(NetworkRoute route,uint32_t elapsed) {
  if (route==NetworkRoute::Home) {
    if (elapsed<120) return 880;
    if (elapsed>=160 && elapsed<280) return 1175;
    if (elapsed>=320 && elapsed<440) return 1568;
  } else if (route==NetworkRoute::AccessPoint) {
    if (elapsed<170) return 440;
    if (elapsed>=240 && elapsed<410) return 330;
  }
  return 0;
}
