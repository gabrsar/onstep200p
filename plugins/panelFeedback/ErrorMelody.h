#pragma once
#include <stdint.h>

// A short descending error motif, inspired by a classic desktop alert.
static constexpr uint32_t errorMelodyDuration=720;
inline unsigned errorMelodyTone(uint32_t elapsed) {
  if (elapsed<150) return 784;
  if (elapsed>=180 && elapsed<330) return 659;
  if (elapsed>=360 && elapsed<690) return 523;
  return 0;
}
