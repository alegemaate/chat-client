#ifndef CHATCOMMON_H
#define CHATCOMMON_H

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

static bool g_bQuit = false;

static SteamNetworkingMicroseconds g_logTimeZero;

void debugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                 const char* pszMsg);

class ChatCommon {
 public:
  void nukeProcess(int rc);

  void fatalError(const char* fmt, ...);

  void printf(const char* fmt, ...);

  void initConnectionSockets();

  void shutdownConnectionSockets();
};

static ChatCommon cc;

#endif  // CHATCOMMON_H
