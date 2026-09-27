#include "../plugins/panelDisplay/StatusEvents.h"
#include <cassert>

int main() {
  StatusEvents events;
  assert(events.observe("", 0) == PanelEvent::None);
  assert(events.observe("nNpAo160", 0) == PanelEvent::Ready);
  assert(events.observe("nNpAo160", 0) == PanelEvent::None);
  assert(events.observe("nNpAo160", 1) == PanelEvent::ClientJoined);
  assert(events.observe("npAo160", 1) == PanelEvent::GotoStarted);
  assert(events.observe("pAo160", 1) == PanelEvent::TrackingStarted);
  assert(events.observe("pAo161", 1) == PanelEvent::MountError);
  assert(events.observe("pAo161", 1) == PanelEvent::None);
  assert(events.observe("pNAo160", 1) == PanelEvent::GotoEnded);
  assert(events.observe("pNAo160", 0) == PanelEvent::ClientLeft);
  assert(events.observe("npNAo160", 0) == PanelEvent::TrackingStopped);
  assert(events.observe("", 0) == PanelEvent::None);
  assert(events.observe("npNAo160", 0) == PanelEvent::Ready);
  assert(events.observe("garbage",0)==PanelEvent::None);
}
