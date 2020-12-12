#pragma once
#include <utility>
#include <algorithm>
#include <gsl/span>
#include <array>
#include <vector>
#include <memory>
#include <chrono>

struct patternbyte
{
    uint8_t value;
    uint8_t mask;

    patternbyte()
    {
        this->clear();
    }

    patternbyte(const uint8_t value, const uint8_t mask)
    {
        this->value = value;
        this->mask = mask;
    }

    bool opaque() const
    {
        return this->mask == 0xffui8;
    }

    void clear()
    {
        this->value = 0ui8;
        this->mask = 0xffui8;
    }

    friend inline bool operator==(
        const patternbyte& left,
        const uint8_t& right)
    {
        return (right & left.mask) == (left.value & left.mask); // value should already be masked
    }

    friend inline bool operator==(
        const uint8_t& left,
        const patternbyte& right)
    {
        return (left & right.mask) == (right.value & right.mask);
    }
};

inline int hexchtoint(char ch)
{
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return -1;
}

static auto compile_pattern(
    const char* pattern)
{
    auto compiled_pattern = std::make_unique<std::vector< patternbyte>>();
    patternbyte b;

    char c;
    auto shift = 0x4UL;
    while (c = *pattern++) {
        if (c == '?' || c == '.') {
            b.mask &= ~(0xf << shift);
        }
        else if (auto num = hexchtoint(c); num != -1) {
            b.value |= (num & 0xf) << shift;
        }
        else {
            continue;
        }

        if (!shift) {
            compiled_pattern->push_back(b);
            b.clear();
        }
        shift ^= 0x4UL;
    }
    if (!shift) {
        b.mask &= 0xf;
        compiled_pattern->push_back(b);
    }
    return compiled_pattern;
}

class pattern_searcher
{
    std::unique_ptr<std::vector< patternbyte>> _cp;

public:
    pattern_searcher(const char* pattern)
    {
        _cp = compile_pattern(pattern);
    }

    template <class Iterator>
    std::pair<Iterator, Iterator> operator()(Iterator first, Iterator last) const
    {
        if (first == last)
            return std::make_pair(last, last);
        if (_cp->empty())
            return std::make_pair(first, first);

        auto it = std::search(first, last, std::begin(*_cp), std::end(*_cp));

        if (it == last)
            return std::make_pair(last, last);
        return std::make_pair(it, it + std::size(*_cp));
    }
};