#include "../config/LxDateReply.h"
#include <cassert>
int main() {
  char reply[80]="1";bool suppress=true;
  assert(lxDateClassicReply("SC",true,false,'I',reply,sizeof(reply),suppress));
  assert(suppress && reply[0]=='1');
  const char *first=strchr(reply,'#');assert(first && first[1]==' ');
  const char *second=strchr(first+1,'#');assert(second && second[1]=='\0');
  assert(strstr(reply,"Updating Planetary Data"));
  strcpy(reply,"1");
  assert(!lxDateClassicReply("SC",true,true,'I',reply,sizeof(reply),suppress));
  assert(!strcmp(reply,"1"));
  assert(!lxDateClassicReply("SC",true,false,'L',reply,sizeof(reply),suppress));
  assert(!lxDateClassicReply("SL",true,false,'I',reply,sizeof(reply),suppress));
  strcpy(reply,"0");
  assert(!lxDateClassicReply("SC",false,false,'I',reply,sizeof(reply),suppress));
  assert(!strcmp(reply,"0"));
  assert(!lxDateClassicReply("SC",true,false,'I',reply,2,suppress));
  assert(!strcmp(reply,"0"));
  assert(lxDateClassicReply("SC",true,false,'A',reply,sizeof(reply),suppress));
}
