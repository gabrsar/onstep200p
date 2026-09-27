#pragma once
#include <stdint.h>
#include <string.h>

enum class PanelEvent : uint8_t {
  None, Ready, ClientJoined, ClientLeft, GotoStarted, GotoEnded,
  TrackingStarted, TrackingStopped, MountError
};

// Observe complete LX200 :GU# snapshots. A completed GoTo is not proof that
// the target is optically centered on this open-loop mount.
class StatusEvents {
public:
  PanelEvent observe(const char *gu, int clients) {
    if (!gu || !*gu) { initialized=false; return PanelEvent::None; }
    const size_t n = strlen(gu);
    if (n < 4 || clients < 0 || gu[n-3]<'0' || gu[n-3]>'9' ||
        gu[n-2]<'0' || gu[n-2]>'9') { initialized=false; return PanelEvent::None; }
    const bool error = gu[n - 1] != '0';
    const bool gotoActive = strchr(gu, 'N') == nullptr;
    const bool tracking = strchr(gu, 'n') == nullptr;
    if (!initialized) {
      initialized = true;
      previousError = error;
      previousGoto = gotoActive;
      previousTracking = tracking;
      previousClients = clients;
      return error ? PanelEvent::MountError : PanelEvent::Ready;
    }
    PanelEvent event = PanelEvent::None;
    if (error) { if (!previousError) event = PanelEvent::MountError; }
    else if (gotoActive != previousGoto)
      event = gotoActive ? PanelEvent::GotoStarted : PanelEvent::GotoEnded;
    else if (clients != previousClients)
      event = clients > previousClients ? PanelEvent::ClientJoined : PanelEvent::ClientLeft;
    else if (tracking != previousTracking)
      event = tracking ? PanelEvent::TrackingStarted : PanelEvent::TrackingStopped;
    previousError = error;
    previousGoto = gotoActive;
    previousTracking = tracking;
    previousClients = clients;
    return event;
  }
private:
  bool initialized = false;
  bool previousError = false;
  bool previousGoto = false;
  bool previousTracking = false;
  int previousClients = 0;
};

inline const char *panelEventText(PanelEvent event) {
  switch (event) {
    case PanelEvent::Ready: return "CONTROLLER ONLINE";
    case PanelEvent::ClientJoined: return "AP DEVICE JOINED";
    case PanelEvent::ClientLeft: return "AP DEVICE LEFT";
    case PanelEvent::GotoStarted: return "GOTO STARTED";
    case PanelEvent::GotoEnded: return "GOTO STOPPED";
    case PanelEvent::TrackingStarted: return "TRACKING ON";
    case PanelEvent::TrackingStopped: return "TRACKING OFF";
    case PanelEvent::MountError: return "MOUNT ERROR";
    default: return "";
  }
}
