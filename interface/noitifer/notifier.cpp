#include "notifier.h"

#ifdef TTF_USE_BOXER
#include <boxer/boxer.h>
#include <cstdio>
#ifdef _WIN32
#include <windows.h>

namespace notify {
bool isTerminal()
{
    // TODO: maybe there's a more reliable check on windows?
    DWORD buffer[2];
    DWORD count = GetConsoleProcessList(buffer, 2);
    if (count == 1) {
          return false;
    }
    return true;
}

}
#else
#include <unistd.h>

namespace notify {
// TODO: maybe MacOS is different?
bool isTerminal()
{
    return isatty(fileno(stdin)) == 1;
}

}

#endif // OS check

#endif // boxer
#include <iostream>

namespace notify {

void notify(const std::string &message)
{
#ifdef TTF_USE_BOXER
    if (!isTerminal()) {
        boxer::show(message.c_str(), "Notification", boxer::Style::Info, boxer::Buttons::OK);
        return;
    }
#endif
    std::cout << message;
}

void warn(const std::string &message)
{
#ifdef TTF_USE_BOXER
    if (!isTerminal()) {
        boxer::show(message.c_str(), "Notification", boxer::Style::Error, boxer::Buttons::OK);
        return;
    }
#endif
    std::cerr << message;
}

} // namespace notify
