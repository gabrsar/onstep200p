#include "../plugins/panelFeedback/ClientFeedback.h"
#include <cassert>
#include <initializer_list>
int main() {
  ClientNotice state;
  assert(!state.connected);
  assert(!state.disconnectedNotice(6000)); // No phantom alert before first client.
  assert(state.update(true,0) && state.connected);
  assert(!state.update(true,10));
  assert(!state.update(false,100));
  assert(!state.disconnectedNotice(5099));
  assert(!state.update(true,200)); // Frequent socket reopening stays quiet.
  assert(!state.update(false,300));
  assert(state.disconnectedNotice(5300));
  assert(!state.disconnectedNotice(5400)); // Once per loss, no repeated alarm.
  assert(state.update(true,5300));
  assert(!state.update(false,0xfffffff0));
  assert(!state.disconnectedNotice(100));
  assert(state.disconnectedNotice(5000));
  assert(state.update(true,6000)); // Wraparound is safe.
  const unsigned notes[]={554,554,494,554,659,554};
  const unsigned starts[]={0,300,600,750,1050,1350};
  for (unsigned i=0;i<6;++i) {
    assert(clientMelodyTone(starts[i])==notes[i]);
    assert(clientMelodyTone(starts[i]+101)==notes[i]);
    assert(clientMelodyTone(starts[i]+102)==0);
  }
  assert(clientMelodyDuration==1800);
  for (unsigned t:{150u,450u,900u,1200u,1500u,1799u,1800u})
    assert(clientMelodyTone(t)==0);
  assert(clientLostTone(0)==330 && clientLostTone(360)==139);
  assert(clientLostTone(80)==0 && clientLostTone(480)==0);
}
