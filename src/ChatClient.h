#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <steam/isteamnetworkingutils.h>
#include <steam/steamnetworkingsockets.h>

/**
 * Chat Client
 *
 * @brief This is the client, which is responsible for communication with the
 * server
 */
class ChatClient : private ISteamNetworkingSocketsCallbacks {
 public:
  void start(const SteamNetworkingIPAddr& serverAddr);

 private:
  HSteamNetConnection m_hConnection;
  ISteamNetworkingSockets* m_pInterface;

  void pollIncomingMessages();

  void pollConnectionStateChanges();

  void pollLocalUserInput();

  virtual void OnSteamNetConnectionStatusChanged(
      SteamNetConnectionStatusChangedCallback_t* pInfo) override;
};

#endif  // CHATCLIENT_H
