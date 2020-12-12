#pragma once

#include <cstdint>
#include <string>
#include <string_view>

template <typename T, T Prime, T OffsetBasis>
struct basic_fnv1a
{
    using type = T;

    struct details
    {
        template <class Char>
        static constexpr Char ascii_tolower(Char c)
        {
            if (c >= 'A' && c <= 'Z')
                return c - ('A' - 'a');
            return c;
        }

        template <class Char>
        static constexpr Char ascii_toupper(Char c)
        {
            if (c >= 'a' && c <= 'z')
                return c - ('a' - 'A');
            return c;
        }
    };

    template <typename Char, typename Traits = std::char_traits<Char>>
    static constexpr T make_hash(
        const Char* s,
        typename Traits::int_type(*fx)(typename Traits::int_type) = nullptr)
    {
        T hash = OffsetBasis;
        for (; *s; ++s) {
            const auto c = fx ? fx(*s) : *s;
            for (std::size_t j = 0; j < sizeof(Char); ++j) {
                hash ^= static_cast<T>((c >> (j * CHAR_BIT)) & 0xff);
                hash *= Prime;
            }
        }
        return hash;
    }

    template <typename Char, typename Traits = std::char_traits<Char>>
    static constexpr T make_hash(
        const Char* s,
        std::size_t size,
        typename Traits::int_type(*fx)(typename Traits::int_type) = nullptr)
    {
        T hash = OffsetBasis;
        for (std::size_t i = 0; i < size; ++i) {
            const auto c = fx ? fx(s[i]) : s[i];
            for (auto j = 0; j < sizeof(Char); ++j) {
                hash ^= static_cast<T>((c >> (j * CHAR_BIT)) & 0xff);
                hash *= Prime;
            }
        }
        return hash;
    }

    template <typename Char, typename Traits>
    static inline constexpr T make_hash(
        const std::basic_string_view<Char, Traits>& s,
        typename Traits::int_type(*fx)(typename Traits::int_type) = nullptr)
    {
        return make_hash(s.data(), s.size(), fx);
    }

    template <typename Char, std::size_t Size, typename Traits = std::char_traits<Char>>
    static inline constexpr T make_hash(
        const std::array<Char, Size>& s,
        typename Traits::int_type(*fx)(typename Traits::int_type) = nullptr)
    {
        return make_hash(s.data(), s.size(), fx);
    }

    template <typename Char, typename Traits, typename Alloc>
    static inline T make_hash(
        const std::basic_string<Char, Traits, Alloc>& s,
        typename Traits::int_type(*fx)(typename Traits::int_type) = nullptr)
    {
        return make_hash(s.c_str(), s.size(), fx);
    }
};

using fnv1a32 = basic_fnv1a<std::uint32_t, 0x1000193UL, 2166136261UL>;
using fnv1a64 = basic_fnv1a<std::uint64_t, 0x100000001b3ULL, 14695981039346656037ULL>;

inline constexpr auto operator"" _fnv1a32(const char* s, std::size_t len)
{
    return fnv1a32::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1a32(const wchar_t* s, std::size_t len)
{
    return fnv1a32::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1a32u(const wchar_t* s, std::size_t len)
{
    return fnv1a32::make_hash(s, len, fnv1a32::details::ascii_toupper);
}
inline constexpr auto operator"" _fnv1a32l(const wchar_t* s, std::size_t len)
{
    return fnv1a32::make_hash(s, len, fnv1a32::details::ascii_tolower);
}
inline constexpr auto operator"" _fnv1a64(const char* s, std::size_t len)
{
    return fnv1a64::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1a64(const wchar_t* s, std::size_t len)
{
    return fnv1a64::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1a64u(const wchar_t* s, std::size_t len)
{
    return fnv1a64::make_hash(s, len, fnv1a64::details::ascii_toupper);
}
inline constexpr auto operator"" _fnv1a64l(const wchar_t* s, std::size_t len)
{
    return fnv1a64::make_hash(s, len, fnv1a64::details::ascii_tolower);
}

#ifdef _M_X64
using fnv1a = fnv1a64;
#else
using fnv1a = fnv1a32;
#endif

inline constexpr auto operator"" _fnv1a(const char* s, std::size_t len)
{
    return fnv1a::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1au(const char* s, std::size_t len)
{
    return fnv1a::make_hash(s, len, fnv1a::details::ascii_toupper);
}
inline constexpr auto operator"" _fnv1al(const char* s, std::size_t len)
{
    return fnv1a::make_hash(s, len, fnv1a::details::ascii_tolower);
}

inline constexpr auto operator"" _fnv1a(const wchar_t* s, std::size_t len)
{
    return fnv1a::make_hash(s, len);
}
inline constexpr auto operator"" _fnv1au(const wchar_t* s, std::size_t len)
{
    return fnv1a::make_hash(s, len, fnv1a::details::ascii_toupper);
}
inline constexpr auto operator"" _fnv1al(const wchar_t* s, std::size_t len)
{
    return fnv1a::make_hash(s, len, fnv1a::details::ascii_tolower);
}