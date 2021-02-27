#include "ChatCommon.h"

#include <chrono>
#include <string>
#include <thread>

#define WIN32

#ifdef WIN32
#include <windows.h>
#else
#include <signal.h>
#include <unistd.h>
#endif

// We do this because I won't want to figure out how to cleanly shut
// down the thread that is reading from stdin.
void ChatCommon::nukeProcess(int rc) {
#ifdef WIN32
  ExitProcess(rc);
#else
  kill(getpid(), SIGKILL);
#endif
}

void debugOutput(ESteamNetworkingSocketsDebugOutputType eType,
                 const char* pszMsg) {
  SteamNetworkingMicroseconds time =
      SteamNetworkingUtils()->GetLocalTimestamp() - g_logTimeZero;
  printf("%10.6f %s\n", time * 1e-6, pszMsg);
  fflush(stdout);
  if (eType == k_ESteamNetworkingSocketsDebugOutputType_Bug) {
    fflush(stdout);
    fflush(stderr);
    cc.nukeProcess(1);
  }
}

void ChatCommon::fatalError(const char* fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char* nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  debugOutput(k_ESteamNetworkingSocketsDebugOutputType_Bug, text);
}

void ChatCommon::printf(const char* fmt, ...) {
  char text[2048];
  va_list ap;
  va_start(ap, fmt);
  vsprintf(text, fmt, ap);
  va_end(ap);
  char* nl = strchr(text, '\0') - 1;
  if (nl >= text && *nl == '\n')
    *nl = '\0';
  debugOutput(k_ESteamNetworkingSocketsDebugOutputType_Msg, text);
}

void ChatCommon::initConnectionSockets() {
  SteamDatagramErrMsg errMsg;
  if (!GameNetworkingSockets_Init(nullptr, errMsg))
    cc.fatalError("GameNetworkingSockets_Init failed.  %s", errMsg);

  g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

  SteamNetworkingUtils()->SetDebugOutputFunction(
      k_ESteamNetworkingSocketsDebugOutputType_Msg, debugOutput);
}

void ChatCommon::shutdownConnectionSockets() {
  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  GameNetworkingSockets_Kill();
}
