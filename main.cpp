#include <thread>
#include <chrono>

#include "ChatCommon.h"
#include "ChatServer.h"
#include "ChatClient.h"


int main( int argc, const char *argv[] ) {
  const uint16 DEFAULT_SERVER_PORT = 27020;

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

		// Anything else, must be server address to connect to
		if (bClient && addrServer.IsIPv6AllZeros()) {
			if (!addrServer.ParseString(argv[i]))
				cc.FatalError("Invalid server address '%s'", argv[i]);
			if (addrServer.m_port == 0)
				addrServer.m_port = DEFAULT_SERVER_PORT;
			continue;
		}

		return 0;
	}

	if (bClient == bServer || (bClient && addrServer.IsIPv6AllZeros()))
		return 0;

	if (bClient) {
		ChatClient client;
		client.Run(addrServer);
	}
	else {
		ChatServer server;
		server.Run((uint16)nPort);
	}


	// Ug, why is there no simple solution for portable, non-blocking console user input?
	// Just nuke the process
	//LocalUserInput_Kill();
	cc.NukeProcess(0);
}
