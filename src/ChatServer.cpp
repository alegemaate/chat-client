#include "ChatServer.h"

#include <assert.h>
#include <chrono>
#include <string>
#include <thread>

#include "ChatCommon.h"

void ChatServer::start(uint16 nPort) {
  // Create client and server sockets
  cc.initConnectionSockets();

  // Restart id index
  id_index = 0;

  // Select instance to use.  For now we'll always use the default.
  // But we could use SteamGameServerNetworkingSockets() on Steam.
  m_pInterface = SteamNetworkingSockets();

  // Start listening
  SteamNetworkingIPAddr serverLocalAddr;
  serverLocalAddr.Clear();
  serverLocalAddr.m_port = nPort;
  m_hListenSock = m_pInterface->CreateListenSocketIP(serverLocalAddr);
  if (m_hListenSock == k_HSteamListenSocket_Invalid)
    cc.fatalError("Failed to listen on port %d", nPort);
  cc.printf("Server listening on port %d\n", nPort);

  while (!g_bQuit) {
    pollIncomingMessages();
    pollConnectionStateChanges();
    pollLocalUserInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Close all the connections
  cc.printf("Closing connections...\n");
  for (auto it : m_mapClients) {
    // Send them one more goodbye message
    sendStringToClient(it.first, "Server is shutting down.  Goodbye.");

    // Close the connection.
    m_pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
  }
  m_mapClients.clear();

  cc.shutdownConnectionSockets();
}

void ChatServer::sendStringToClient(HSteamNetConnection conn, const char* str) {
  m_pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str),
                                        k_nSteamNetworkingSend_Reliable);
}

void ChatServer::sendStringToAllClients(const char* str,
                                        HSteamNetConnection except) {
  for (auto& c : m_mapClients) {
    if (c.first != except)
      sendStringToClient(c.first, str);
  }
}

void ChatServer::pollIncomingMessages() {
  char temp[1024];

  while (!g_bQuit) {
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnListenSocket(m_hListenSock,
                                                              &pIncomingMsg, 1);
    if (numMsgs == 0)
      break;
    if (numMsgs < 0)
      cc.fatalError("Error checking for messages");
    assert(numMsgs == 1 && pIncomingMsg);
    auto itClient = m_mapClients.find(pIncomingMsg->m_conn);
    assert(itClient != m_mapClients.end());

    // '\0'-terminate it to make it easier to parse
    std::string sCmd;
    sCmd.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
    const char* cmd = sCmd.c_str();

    // We don't need this anymore.
    pIncomingMsg->Release();

    // Check for known commands.  None of this example code is secure or robust.
    // Don't write a real server like this, please.
    sprintf(temp, "Message from client %d (%s)", itClient->second.m_sID, cmd);
  }
}

void ChatServer::pollConnectionStateChanges() {
  m_pInterface->RunCallbacks(this);
}

void ChatServer::pollLocalUserInput() {
  /*
  std::string cmd;
  while ( !g_bQuit && lui.GetNext( cmd )) {

    if ( strcmp( cmd.c_str(), "/quit" ) == 0 ) {
      g_bQuit = true;
      cc.printf( "Shutting down server" );
      break;
    }
  }
  */
}

void ChatServer::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* pInfo) {
  char temp[1024];

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
      // NOTE: We will get callbacks here when we destroy connections.  You can
      // ignore these.
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      // Ignore it they were not previously connected.  (If they disconnected
      // before we accepted the connection.)
      if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected) {
        // Locate the client.
        auto itClient = m_mapClients.find(pInfo->m_hConn);
        assert(itClient != m_mapClients.end());

        // Select appropriate log messages
        const char* pszDebugLogAction;
        if (pInfo->m_info.m_eState ==
            k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
          pszDebugLogAction = "problem detected locally";
          sprintf(temp, "Client %d disconnected.  (%s)", itClient->second.m_sID,
                  pInfo->m_info.m_szEndDebug);
        } else {
          pszDebugLogAction = "closed by peer";
          sprintf(temp, "Client %d disconnected.", itClient->second.m_sID);
        }

        // Spew something to our own log.
        cc.printf("Connection %s %s, reason %d: %s\n",
                  pInfo->m_info.m_szConnectionDescription, pszDebugLogAction,
                  pInfo->m_info.m_eEndReason, pInfo->m_info.m_szEndDebug);

        m_mapClients.erase(itClient);

        // Send a message so everybody else knows what happened
        sendStringToAllClients(temp);
      } else {
        assert(pInfo->m_eOldState ==
               k_ESteamNetworkingConnectionState_Connecting);
      }

      // Clean up the connection.
      m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
      break;
    }

    case k_ESteamNetworkingConnectionState_Connecting: {
      // This must be a new connection
      assert(m_mapClients.find(pInfo->m_hConn) == m_mapClients.end());
      cc.printf("Connection request from %s",
                pInfo->m_info.m_szConnectionDescription);

      // A client is attempting to connect
      // Try to accept the connection.
      if (m_pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK) {
        m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
        cc.printf("Can't accept connection.  (It was already closed?)");
        break;
      }

      // Send them a welcome message
      sprintf(temp, "Connected to server, id:%d.", id_index);
      sendStringToClient(pInfo->m_hConn, temp);

      // Also send them a list of everybody who is already connected
      sprintf(temp, "%d other clients connected.", (int)m_mapClients.size());

      // Let everybody else know who they are for now
      sprintf(temp, "Client connected with id:%d", id_index);
      sendStringToAllClients(temp, pInfo->m_hConn);

      // Add them to the client list, using std::map wacky syntax
      m_mapClients[pInfo->m_hConn];
      m_mapClients[pInfo->m_hConn].m_sID = id_index;
      m_pInterface->SetConnectionName(pInfo->m_hConn,
                                      std::to_string(id_index++).c_str());

      break;
    }

    case k_ESteamNetworkingConnectionState_Connected:
      break;

    default:
      break;
  }
}
