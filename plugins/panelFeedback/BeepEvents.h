#pragma once
#include <stdint.h>
enum class BeepEvent : uint8_t { Boot, Ready, TimeReady, Joined, Left, GotoStart, GotoEnd, Sync, Error, Click, WifiHome, WifiAccessPoint, CalStep, CalDone, ClientConnected, ClientDisconnected };
inline uint8_t beepPulses(BeepEvent event) {
  return event==BeepEvent::Error || event==BeepEvent::Sync?3:
    event==BeepEvent::Ready || event==BeepEvent::Joined || event==BeepEvent::GotoEnd || event==BeepEvent::CalDone?2:1;
}
