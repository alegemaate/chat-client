#ifndef CHATCOMMON_H
#define CHATCOMMON_H

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

static bool g_bQuit = false;

static SteamNetworkingMicroseconds g_logTimeZero;

void DebugOutput( ESteamNetworkingSocketsDebugOutputType eType, const char *pszMsg );

class ChatCommon {
  public:
    void NukeProcess( int rc );

    void FatalError( const char *fmt, ... );

    void Printf( const char *fmt, ... );
};

static ChatCommon cc;

#endif // CHATCOMMON_H
