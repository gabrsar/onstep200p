#pragma once
#include <stdint.h>

// Single click waits for the triple-click window; held buttons never repeat.
// A pending gesture blocks all mount commands.
class ClickGesture {
public:
  bool update(bool down, uint32_t now) {
    if (count && !raw && !stable && now-lastPress>450) {
      single=count==1; count=0;
    }
    if (down != raw) { raw=down; changed=now; }
    if (raw != stable && now-changed >= 30) {
      stable=raw;
      if (stable) {
        if (count && now-lastPress>450) count=0;
        heldAt=now;
        lastPress=now;
        if (++count == 3) { count=0; return true; }
      } else if (now-heldAt>=1500) { count=0; single=false; }
    }
    return false;
  }
  bool blocked() const { return raw || stable || count; }
  uint8_t presses() const { return count; }
  bool takeSingle() { const bool value=single; single=false; return value; }
private:
  bool raw=false, stable=false, single=false;
  uint8_t count=0;
  uint32_t changed=0, lastPress=0, heldAt=0;
};
