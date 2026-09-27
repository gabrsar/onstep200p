#include "../plugins/panelFeedback/NetworkFeedback.h"
#include <cassert>
int main() {
  NetworkNotice boot;
  assert(boot.update(false,true,true,0,false)==NetworkRoute::Off);
  assert(boot.update(false,true,true,7999,true)==NetworkRoute::Off);
  // A prompt home association suppresses the transient AP announcement.
  assert(boot.update(true,true,true,8000,true)==NetworkRoute::Off);
  assert(boot.update(true,true,true,9000,true)==NetworkRoute::Home);
  assert(boot.update(true,true,true,20000,true)==NetworkRoute::Off);
  // Lost home: AP fallback is announced once after grace, not on every poll.
  boot.update(false,true,true,21000,true);
  assert(boot.update(false,true,true,29000,true)==NetworkRoute::AccessPoint);
  assert(boot.update(false,true,true,40000,true)==NetworkRoute::Off);
  boot.update(true,true,true,41000,false);
  assert(boot.update(true,true,true,42000,false)==NetworkRoute::Off);
  assert(boot.update(true,true,true,43000,true)==NetworkRoute::Home);

  NetworkNotice apOnly;
  apOnly.update(false,true,false,100,true);
  assert(apOnly.update(false,true,false,1100,true)==NetworkRoute::AccessPoint);
  // Only the newest state survives while higher-priority audio is playing.
  apOnly.update(true,true,true,1200,false);
  apOnly.update(false,true,true,2300,false);
  assert(apOnly.update(false,true,true,12000,true)==NetworkRoute::Off);

  NetworkNotice wrap;
  wrap.update(true,true,true,0xfffffff0,true);
  assert(wrap.update(true,true,true,1000,true)==NetworkRoute::Home);
  assert(networkMelodyTone(NetworkRoute::Home,0)==880);
  assert(networkMelodyTone(NetworkRoute::Home,160)==1175);
  assert(networkMelodyTone(NetworkRoute::Home,320)==1568);
  assert(networkMelodyTone(NetworkRoute::AccessPoint,0)==440);
  assert(networkMelodyTone(NetworkRoute::AccessPoint,240)==330);
  assert(networkMelodyTone(NetworkRoute::Home,480)==0);
  assert(networkMelodyTone(NetworkRoute::Off,0)==0);
}
