#include <thread>
#include <chrono>

#include "ChatCommon.h"
#include "ChatServer.h"
#include "ChatClient.h"

const uint16 DEFAULT_SERVER_PORT = 27020;

static void InitSteamDatagramConnectionSockets() {
  SteamDatagramErrMsg errMsg;
  if ( !GameNetworkingSockets_Init( nullptr, errMsg ) )
    cc.FatalError( "GameNetworkingSockets_Init failed.  %s", errMsg );

	g_logTimeZero = SteamNetworkingUtils()->GetLocalTimestamp();

	SteamNetworkingUtils()->SetDebugOutputFunction( k_ESteamNetworkingSocketsDebugOutputType_Msg, DebugOutput );
}

static void ShutdownSteamDatagramConnectionSockets() {
	std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );

  GameNetworkingSockets_Kill();
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
}
