#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <string>
#include <map>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>

class ChatClient : private ISteamNetworkingSocketsCallbacks {
  public:
    void Run( const SteamNetworkingIPAddr &serverAddr );

  private:

    HSteamNetConnection m_hConnection;
    ISteamNetworkingSockets *m_pInterface;

    void PollIncomingMessages();

    void PollConnectionStateChanges();

    void PollLocalUserInput();

    virtual void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *pInfo) override;

};

#endif // CHATCLIENT_H
