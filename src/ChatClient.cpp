#include "ChatClient.h"

#include <assert.h>
#include <chrono>
#include <string>
#include <thread>

#include "ChatCommon.h"

// Start it up!
void ChatClient::start(const SteamNetworkingIPAddr& serverAddr) {
  // Create client and server sockets
  cc.initConnectionSockets();

  // Select instance to use.  For now we'll always use the default.
  m_pInterface = SteamNetworkingSockets();

  // Start connecting
  char szAddr[SteamNetworkingIPAddr::k_cchMaxString];
  serverAddr.ToString(szAddr, sizeof(szAddr), true);
  cc.printf("Connecting to chat server at %s", szAddr);

  m_hConnection = m_pInterface->ConnectByIPAddress(serverAddr);
  if (m_hConnection == k_HSteamNetConnection_Invalid) {
    cc.fatalError("Failed to create connection");
  }

  while (!g_bQuit) {
    pollIncomingMessages();
    pollConnectionStateChanges();
    pollLocalUserInput();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  cc.shutdownConnectionSockets();
}

void ChatClient::pollIncomingMessages() {
  while (!g_bQuit) {
    ISteamNetworkingMessage* pIncomingMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnConnection(m_hConnection,
                                                            &pIncomingMsg, 1);
    if (numMsgs == 0)
      break;
    if (numMsgs < 0)
      cc.fatalError("Error checking for messages");

    // Just echo anything we get from the server
    fwrite(pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout);
    fputc('\n', stdout);

    // We don't need this anymore.
    pIncomingMsg->Release();
  }
}

void ChatClient::pollConnectionStateChanges() {
  m_pInterface->RunCallbacks(this);
}

void ChatClient::pollLocalUserInput() {
  /*std::string cmd;
  while (!g_bQuit && lui.GetNext(cmd)) {

    // Check for known commands
    if ( strcmp( cmd.c_str(), "/quit" ) == 0 ) {
      g_bQuit = true;
      cc.printf( "Disconnecting from server" );

      // Close the connection gracefully.
      // We use linger mode to ask for any remaining reliable data
      // to be flushed out.  But remember this is an application
      // protocol on UDP.  See shutdownConnectionSockets
      m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
      break;
    }

    // Anything else, just send it to the server and let them parse it
    m_pInterface->SendMessageToConnection( m_hConnection, cmd.c_str(),
  (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable );
  }*/
}

void ChatClient::OnSteamNetConnectionStatusChanged(
    SteamNetConnectionStatusChangedCallback_t* pInfo) {
  assert(pInfo->m_hConn == m_hConnection ||
         m_hConnection == k_HSteamNetConnection_Invalid);

  // What's the state of the connection?
  switch (pInfo->m_info.m_eState) {
    case k_ESteamNetworkingConnectionState_None:
      // NOTE: We will get callbacks here when we destroy connections.  You can
      // ignore these.
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally: {
      g_bQuit = true;

      // Print an appropriate message
      if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting) {
        // Note: we could distinguish between a timeout, a rejected connection,
        // or some other transport problem.
        cc.printf("Could not connect.  (%s)", pInfo->m_info.m_szEndDebug);
      } else if (pInfo->m_info.m_eState ==
                 k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
        cc.printf("Lost contact with host.  (%s)", pInfo->m_info.m_szEndDebug);
      } else {
        // NOTE: We could check the reason code for a normal disconnection
        cc.printf("Server shut down.  (%s)", pInfo->m_info.m_szEndDebug);
      }

      // Clean up the connection.
      m_pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
      m_hConnection = k_HSteamNetConnection_Invalid;
      break;
    }

    case k_ESteamNetworkingConnectionState_Connecting:
      break;

    case k_ESteamNetworkingConnectionState_Connected:
      cc.printf("Connected to server OK");
      break;

    default:
      break;
  }
}
