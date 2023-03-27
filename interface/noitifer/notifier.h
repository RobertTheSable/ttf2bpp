#ifndef TTF2BPP_NOTIFIER_H
#define TTF2BPP_NOTIFIER_H

#include <string>

namespace notify
{
    bool isTerminal();
    void notify(const std::string& message);
    void warn(const std::string& message);
}

#endif // TTF2BPP_NOTIFIER_H
