#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <string>
#include <map>

#include <steam/steamnetworkingsockets.h>
#include <steam/isteamnetworkingutils.h>
#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
#include <steam/steam_api.h>
#endif

/////////////////////////////////////////////////////////////////////////////
//
// ChatServer
//
/////////////////////////////////////////////////////////////////////////////

class ChatServer : private ISteamNetworkingSocketsCallbacks {
  public:
    void Run( uint16 nPort );

  private:

    HSteamListenSocket m_hListenSock;
    ISteamNetworkingSockets *m_pInterface;

    struct Client_t
    {
      std::string m_sNick;
    };

    std::map< HSteamNetConnection, Client_t > m_mapClients;

    void SendStringToClient( HSteamNetConnection conn, const char *str );

    void SendStringToAllClients( const char *str, HSteamNetConnection except = k_HSteamNetConnection_Invalid );

    void PollIncomingMessages();

    void PollConnectionStateChanges();

    void PollLocalUserInput();

    void SetClientNick( HSteamNetConnection hConn, const char *nick );

    virtual void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo ) override;
};

#endif // CHATSERVER_H
