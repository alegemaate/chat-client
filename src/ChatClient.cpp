#include "ChatClient.h"

#include <assert.h>
#include <thread>
#include <chrono>

#include "ChatCommon.h"
#include "LocalUserInput.h"

void ChatClient::Run( const SteamNetworkingIPAddr &serverAddr ) {
  // Select instance to use.  For now we'll always use the default.
  m_pInterface = SteamNetworkingSockets();

  // Start connecting
  char szAddr[ SteamNetworkingIPAddr::k_cchMaxString ];
  serverAddr.ToString( szAddr, sizeof(szAddr), true );
  cc.Printf( "Connecting to chat server at %s", szAddr );
  m_hConnection = m_pInterface->ConnectByIPAddress( serverAddr );
  if ( m_hConnection == k_HSteamNetConnection_Invalid )
    cc.FatalError( "Failed to create connection" );

  while ( !g_bQuit )
  {
    PollIncomingMessages();
    PollConnectionStateChanges();
    PollLocalUserInput();
    std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) );
  }
}


void ChatClient::PollIncomingMessages() {
  while ( !g_bQuit )
  {
    ISteamNetworkingMessage *pIncomingMsg = nullptr;
    int numMsgs = m_pInterface->ReceiveMessagesOnConnection( m_hConnection, &pIncomingMsg, 1 );
    if ( numMsgs == 0 )
      break;
    if ( numMsgs < 0 )
      cc.FatalError( "Error checking for messages" );

    // Just echo anything we get from the server
    fwrite( pIncomingMsg->m_pData, 1, pIncomingMsg->m_cbSize, stdout );
    fputc( '\n', stdout );

    // We don't need this anymore.
    pIncomingMsg->Release();
  }
}

void ChatClient::PollConnectionStateChanges() {
  m_pInterface->RunCallbacks( this );
}

void ChatClient::PollLocalUserInput() {
  std::string cmd;
  while ( !g_bQuit && lui.GetNext( cmd )) {

    // Check for known commands
    if ( strcmp( cmd.c_str(), "/quit" ) == 0 ) {
      g_bQuit = true;
      cc.Printf( "Disconnecting from server" );

      // Close the connection gracefully.
      // We use linger mode to ask for any remaining reliable data
      // to be flushed out.  But remember this is an application
      // protocol on UDP.  See ShutdownSteamDatagramConnectionSockets
      m_pInterface->CloseConnection( m_hConnection, 0, "Goodbye", true );
      break;
    }

    // Anything else, just send it to the server and let them parse it
    m_pInterface->SendMessageToConnection( m_hConnection, cmd.c_str(), (uint32)cmd.length(), k_nSteamNetworkingSend_Reliable );
  }
}

void ChatClient::OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t *pInfo ) {
  assert( pInfo->m_hConn == m_hConnection || m_hConnection == k_HSteamNetConnection_Invalid );

  // What's the state of the connection?
  switch ( pInfo->m_info.m_eState )
  {
    case k_ESteamNetworkingConnectionState_None:
      // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
      break;

    case k_ESteamNetworkingConnectionState_ClosedByPeer:
    case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
    {
      g_bQuit = true;

      // Print an appropriate message
      if ( pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting )
      {
        // Note: we could distinguish between a timeout, a rejected connection,
        // or some other transport problem.
        cc.Printf( "Could not connect.  (%s)", pInfo->m_info.m_szEndDebug );
      }
      else if ( pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally )
      {
        cc.Printf( "Lost contact with host.  (%s)", pInfo->m_info.m_szEndDebug );
      }
      else
      {
        // NOTE: We could check the reason code for a normal disconnection
        cc.Printf( "Server shut down.  (%s)", pInfo->m_info.m_szEndDebug );
      }

      // Clean up the connection.  This is important!
      // The connection is "closed" in the network sense, but
      // it has not been destroyed.  We must close it on our end, too
      // to finish up.  The reason information do not matter in this case,
      // and we cannot linger because it's already closed on the other end,
      // so we just pass 0's.
      m_pInterface->CloseConnection( pInfo->m_hConn, 0, nullptr, false );
      m_hConnection = k_HSteamNetConnection_Invalid;
      break;
    }

    case k_ESteamNetworkingConnectionState_Connecting:
      // We will get this callback when we start connecting.
      // We can ignore this.
      break;

    case k_ESteamNetworkingConnectionState_Connected:
      cc.Printf( "Connected to server OK" );
      break;

    default:
      // Silences -Wswitch
      break;
  }
}
