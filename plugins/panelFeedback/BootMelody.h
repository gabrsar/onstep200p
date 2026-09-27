#pragma once
#include <stdint.h>

// Shave and a Haircut: TA ta-ta TA TA ... TA TA.
// Slots include a short gap; seven notes plus the deliberate answering pause.
static constexpr uint32_t bootMelodyDuration=800;
static constexpr uint32_t readyMelodyDuration=460;
inline unsigned bootMelodyTone(uint32_t elapsed) {
  const unsigned hz[]={523,392,392,440,392};
  const uint16_t slot[]={200,100,100,200,200};
  for (int i=0;i<5;++i) {
    if (elapsed<slot[i]) return elapsed<slot[i]-25u?hz[i]:0;
    elapsed-=slot[i];
  }
  return 0;
}
inline unsigned readyMelodyTone(uint32_t elapsed) {
  if (elapsed<175) return 494;
  if (elapsed>=200 && elapsed<435) return 523;
  return 0;
}
