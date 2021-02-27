#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <map>

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

class ChatServer : private ISteamNetworkingSocketsCallbacks {
 public:
  void start(uint16 nPort);

 private:
  HSteamListenSocket m_hListenSock;
  ISteamNetworkingSockets* m_pInterface;

  struct Client_t {
    int m_sID;
  };

  std::map<HSteamNetConnection, Client_t> m_mapClients;

  void sendStringToClient(HSteamNetConnection conn, const char* str);

  void sendStringToAllClients(
      const char* str,
      HSteamNetConnection except = k_HSteamNetConnection_Invalid);

  void pollIncomingMessages();

  void pollConnectionStateChanges();

  void pollLocalUserInput();

  virtual void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* pInfo) override;

  int id_index;
};

#endif  // CHATSERVER_H
