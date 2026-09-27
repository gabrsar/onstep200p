#include <cassert>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
#define ON 1
#define SERIAL_LOCAL_MODE ON
#define VF(x) ((void)0)
#define VLF(x) ((void)0)
#define sstrcpy(a,b) strcpy(a,b)
#define sstrcpyex(a,b,n) snprintf(a,n,"%s",b)
unsigned long millis() { return 0; }
void noInterrupts() {}
void interrupts() {}
struct Tasks {
  bool add(int,int,bool,int,void(*)(),const char*) { return true; }
} tasks;
struct SerialStub {
  std::vector<std::string> sent;
  char empty[1]={0};
  char *receive() { return empty; }
  bool receiveAvailable() { return false; }
  void transmit(const char *text) { sent.push_back(text); }
} serialStub;
#define SERIAL_LOCAL serialStub
