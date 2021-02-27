#include <iostream>

#include "ChatClient.h"
#include "ChatCommon.h"
#include "ChatServer.h"

const uint16 DEFAULT_SERVER_PORT = 27020;

int main(int argc, const char* argv[]) {
  // No cli params
  if (argc < 2) {
    std::cout << "Please run arguments <client|server> [address]" << std::endl;
    return 1;
  }

  // Server mode
  if (!strcmp(argv[1], "server")) {
    std::cout << "Starting Client" << std::endl;

    // Start server here
    ChatServer server;
    server.start(DEFAULT_SERVER_PORT);
  }

  // Client mode
  else if (!strcmp(argv[1], "client")) {
    // Validate client params
    if (argc < 3) {
      std::cout << "Client must be provided with IP to connect to."
                << std::endl;
      return 1;
    }

    SteamNetworkingIPAddr addrServer;
    addrServer.Clear();

    if (!addrServer.ParseString(argv[2])) {
      std::cout << "Invalid server address '" << argv[2] << "'" << std::endl;
      return 1;
    }

    if (addrServer.m_port == 0) {
      addrServer.m_port = DEFAULT_SERVER_PORT;
    }

    ChatClient client;
    client.start(addrServer);
  }

  // Nuke the process
  cc.nukeProcess(0);

  return 0;
}
