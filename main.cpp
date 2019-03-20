#include <thread>
#include <chrono>

#include "ChatCommon.h"
#include "ChatServer.h"
#include "ChatClient.h"

const uint16 DEFAULT_SERVER_PORT = 27020;

static void InitSteamDatagramConnectionSockets()
{
	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		SteamDatagramErrMsg errMsg;
		if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
			cc.FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );
	#else
		SteamDatagramClient_SetAppIDAndUniverse( 570, k_EUniverseDev ); // Just set something, doesn't matter what

		SteamDatagramErrMsg errMsg;
		if ( !SteamDatagramClient_Init( true, errMsg ) )
			cc.FatalError( "SteamDatagramClient_Init failed.  %s", errMsg );

		// Disable authentication when running with Steam, for this
		// example, since we're not a real app.
		//
		// Authentication is disabled automatically in the open-source
		// version since we don't have a trusted third party to issue
		// certs.
		SteamNetworkingUtils()->SetGlobalConfigValueInt32( k_ESteamNetworkingConfig_IP_AllowWithoutAuth, 1 );
	#endif

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
}

static void ShutdownSteamDatagramConnectionSockets()
{
	// Give connections time to finish up.  This is an application layer protocol
	// here, it's not TCP.  Note that if you have an application and you need to be
	// more sure about cleanup, you won't be able to do this.  You will need to send
	// a message and then either wait for the peer to close the connection, or
	// you can pool the connection to see if any reliable data is pending.
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

	#ifdef STEAMNETWORKINGSOCKETS_OPENSOURCE
		GameNetworkingSockets_Kill();
	#else
		SteamDatagramClient_Kill();
	#endif
}


void PrintUsageAndExit( int rc = 1 ) {
	fflush(stderr);
	printf(
R"usage(Usage:
    example_chat client SERVER_ADDR
    example_chat server [--port PORT]
)usage"
	);
	fflush(stdout);
	exit(rc);
}

int main( int argc, const char *argv[] ) {
	bool bServer = false;
	bool bClient = false;
	int nPort = DEFAULT_SERVER_PORT;
	SteamNetworkingIPAddr addrServer;
	addrServer.Clear();

	for (int i = 1 ; i < argc ; ++i) {
		if (!bClient && !bServer) {
			if (!strcmp( argv[i], "client" )) {
				bClient = true;
				continue;
			}
			if (!strcmp( argv[i], "server" )) {
				bServer = true;
				continue;
			}
		}
		if (!strcmp( argv[i], "--port" )) {
			++i;
			if (i >= argc)
				PrintUsageAndExit();
			nPort = atoi( argv[i] );
			if (nPort <= 0 || nPort > 65535)
				cc.FatalError( "Invalid port %d", nPort );
			continue;
		}

		// Anything else, must be server address to connect to
		if (bClient && addrServer.IsIPv6AllZeros()) {
			if (!addrServer.ParseString(argv[i]))
				cc.FatalError("Invalid server address '%s'", argv[i]);
			if (addrServer.m_port == 0)
				addrServer.m_port = DEFAULT_SERVER_PORT;
			continue;
		}

		PrintUsageAndExit();
	}

	if (bClient == bServer || (bClient && addrServer.IsIPv6AllZeros()))
		PrintUsageAndExit();

	// Create client and server sockets
	InitSteamDatagramConnectionSockets();

	if (bClient) {
		ChatClient client;
		client.Run(addrServer);
	}
	else {
		ChatServer server;
		server.Run((uint16)nPort);
	}

	ShutdownSteamDatagramConnectionSockets();

	// Ug, why is there no simple solution for portable, non-blocking console user input?
	// Just nuke the process
	//LocalUserInput_Kill();
	cc.NukeProcess(0);

	return 0;
}
