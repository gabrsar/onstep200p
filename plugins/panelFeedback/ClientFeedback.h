#pragma once
#include <stdint.h>
// Suppress repeated paging for clients that frequently reopen their socket.
class ClientNotice {
public:
  bool connected=false;
  bool update(bool value,uint32_t now) {
    if (value==connected) return false;
    connected=value;
    if (!value) { leftAt=now; offlineReported=false; return false; }
    const bool announce=!seen || now-leftAt>=5000;
    seen=true; return announce;
  }
  bool disconnectedNotice(uint32_t now) {
    if (connected || !seen || offlineReported || now-leftAt<5000) return false;
    offlineReported=true; return true;
  }
private:
  bool seen=false;
  bool offlineReported=true;
  uint32_t leftAt=0;
};
// Communicator cadence: notes at 0,300,600,750,1050,1350ms.
// C#5 C#5 B4 C#5 E5 C#5; keep pitches but not equal spacing.
static constexpr uint32_t clientMelodyDuration=1800;
inline unsigned clientMelodyTone(uint32_t elapsed) {
  const unsigned slots[]={554,0,554,0,494,554,0,659,0,554,0,0};
  const unsigned slot=elapsed/150;
  return slot<12 && elapsed%150<102?slots[slot]:0;
}
static constexpr uint32_t clientLostDuration=480;
inline unsigned clientLostTone(uint32_t elapsed) {
  const unsigned notes[]={330,247,208,139}; // E4 B3 G#3 C#3: clear descending cue.
  const unsigned slot=elapsed/120;
  return slot<4 && elapsed%120<80?notes[slot]:0;
}
