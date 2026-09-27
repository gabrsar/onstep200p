#pragma once
#include <stddef.h>
#include <string.h>

// Classic Meade SC success: boolean + two #-terminated status strings.
// Local broker/checksummed OnStep requests retain their single-value reply.
inline bool lxDateClassicReply(const char *command,bool success,bool checksum,
                              char channel,char *reply,size_t capacity,bool &suppressFrame) {
  if (strcmp(command,"SC") || !success || checksum || channel=='L') return false;
  static const char text[]="1Updating Planetary Data#                        #";
  if (capacity<sizeof(text)) return false;
  memcpy(reply,text,sizeof(text));
  suppressFrame=true; // Both terminators already exist; never append a third.
  return true;
}
