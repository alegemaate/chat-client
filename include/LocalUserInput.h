#ifndef LOCALUSERINPUT_H
#define LOCALUSERINPUT_H

#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <algorithm>

// You really gotta wonder what kind of pedantic garbage was
// going through the minds of people who designed std::string
// that they decided not to include trim.
// https://stackoverflow.com/questions/216823/whats-the-best-way-to-trim-stdstring

// trim from start (in place)
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

class LocalUserInput {
  public:
    LocalUserInput();
    virtual ~LocalUserInput();

    // Read the next line of input from stdin, if anything is available.
    bool GetNext( std::string &result );

    void Init();

    void Kill();

    void PollInput();
  protected:

  private:
    static std::mutex mutexUserInputQueue;
    static std::queue< std::string > queueUserInput;

    std::thread *s_pThreadUserInput = nullptr;
};

static LocalUserInput lui;

#endif // LOCALUSERINPUT_H
