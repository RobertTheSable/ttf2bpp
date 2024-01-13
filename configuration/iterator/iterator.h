#ifndef ITERATOR_BREAK_H
#define ITERATOR_BREAK_H

#include <memory>
#include <stdexcept>
#include <unicode/brkiter.h>
#include <unicode/locid.h>

namespace adapter {

struct BoundsException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

class BreakIterator
{
    icu::UnicodeString _u16Data;
    std::unique_ptr<icu::BreakIterator> _itr = nullptr;
    bool _word;
    int32_t _begin, _cur, _next;
public:
    BreakIterator(bool word, const std::string& data, const icu::Locale& locale);
    BreakIterator& operator=(BreakIterator rhs);
    BreakIterator(const BreakIterator& rhs);

    bool done();
    bool atStart();

    BreakIterator& operator++();
    BreakIterator operator++(int);
    BreakIterator& operator--();
    std::string operator*();

    UChar32 ufront() const;
    std::string front() const;
};

}

#endif // ITERATOR_BREAK
