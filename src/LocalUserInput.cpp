#include "LocalUserInput.h"

#include "ChatCommon.h"

#include <iostream>

std::mutex LocalUserInput::mutexUserInputQueue;
std::queue< std::string > LocalUserInput::queueUserInput;

LocalUserInput::LocalUserInput()
{
  //ctor
}

LocalUserInput::~LocalUserInput()
{
  //dtor
}


// Read the next line of input from stdin, if anything is available.
bool LocalUserInput::GetNext( std::string &result ) {
  bool got_input = false;
  mutexUserInputQueue.lock();
  while ( !queueUserInput.empty() && !got_input )
  {
    result = queueUserInput.front();
    queueUserInput.pop();
    ltrim(result);
    rtrim(result);
    got_input = !result.empty(); // ignore blank lines
  }
  mutexUserInputQueue.unlock();
  return got_input;
}

void LocalUserInput::Init() {
  s_pThreadUserInput = new std::thread(&LocalUserInput::PollInput, this);
}

void LocalUserInput::Kill() {
// Does not work.  We won't clean up, we'll just nuke the process.
//	g_bQuit = true;
//	_close( fileno( stdin ) );
//
//	if ( s_pThreadUserInput )
//	{
//		s_pThreadUserInput->join();
//		delete s_pThreadUserInput;
//		s_pThreadUserInput = nullptr;
//	}
}

void LocalUserInput::PollInput() {
  while (!g_bQuit) {
    char szLine[ 4000 ];
    if (!fgets(szLine, sizeof(szLine), stdin)) {
      // Well, you would hope that you could close the handle
      // from the other thread to trigger this.  Nope.
      if (g_bQuit)
        return;
      g_bQuit = true;
      cc.Printf( "Failed to read on stdin, quitting\n" );
      break;
    }

    mutexUserInputQueue.lock();
    queueUserInput.push( std::string( szLine ) );
    mutexUserInputQueue.unlock();
  }
}
