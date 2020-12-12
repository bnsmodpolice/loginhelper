#pragma once
#include <ntdll.h>
#include <locale>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <SafeInt.hpp>

namespace ntapi
{
  template<class T>
  class basic_string : public T
  {
  public:
#pragma region Member types
    using char_type = typename std::remove_pointer_t<decltype(T::Buffer)>;

    using pointer = char_type *;
    using const_pointer = const char_type *;
    using reference = char_type &;
    using const_reference = const char_type &;
    using iterator = pointer;
    using const_iterator = const_pointer;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using size_type = typename decltype(T::Length);
    using difference_type = typename std::make_signed_t<size_type>;
#pragma endregion

    static constexpr size_type npos = size_type(-1);

  public:
#pragma region Constructors
    constexpr basic_string(const_pointer s)
    {
      this->Length = s ? size_type(std::char_traits<char_type>::length(s) * sizeof(char_type)) : 0;
      this->MaximumLength = this->Length;
      this->Buffer = const_cast<pointer>(s);
    }

    constexpr basic_string(const_pointer s, size_type count) noexcept
    {
      this->Length = count * sizeof(char_type);
      this->MaximumLength = this->Length;
      this->Buffer = const_cast<pointer>(s);
    }
#pragma endregion

#pragma region Iterators
#pragma region Const
    constexpr const_iterator begin() const noexcept
    {
      return const_iterator(this->Buffer);
    }

    constexpr const_iterator cbegin() const noexcept
    {
      return const_iterator(this->Buffer);
    }

    constexpr const_iterator end() const noexcept
    {
      return const_iterator(reinterpret_cast<uintptr_t>(this->Buffer) + size_bytes());
    }

    constexpr const_iterator cend() const noexcept
    {
      return const_iterator(reinterpret_cast<uintptr_t>(this->Buffer) + size_bytes());
    }

    constexpr const_reverse_iterator rbegin() const noexcept
    {
      return std::make_reverse_iterator(end());
    }

    constexpr const_reverse_iterator crbegin() const noexcept
    {
      return std::make_reverse_iterator(cend());
    }

    constexpr const_reverse_iterator rend() const noexcept
    {
      return std::make_reverse_iterator(begin());
    }

    constexpr const_reverse_iterator crend() const noexcept
    {
      return std::make_reverse_iterator(cbegin());
    }
#pragma endregion
#pragma endregion Iterators

#pragma region Element access
#pragma region Const
    constexpr const_reference operator[](size_type pos) const
    {
      return this->Buffer[pos];
    }

    constexpr const_reference at(size_type pos) const
    {
      if ( pos >= size() )
        throw std::out_of_range("pos is greater than size");

      return operator[](pos);
    }

    constexpr const_reference front() const
    {
      return operator[](0);
    }

    constexpr const_reference back() const
    {
      return operator[](size() - 1);
    }

    constexpr const_pointer data() const noexcept
    {
      return this->Buffer;
    }
#pragma endregion
#pragma endregion

#pragma region Capacity
    constexpr size_type size_bytes() const noexcept
    {
      return this->Length;
    }

    constexpr size_type length_bytes() const noexcept
    {
      return this->Length;
    }

    constexpr size_type capacity_bytes() const noexcept
    {
      return this->MaximumLength;
    }

    constexpr size_type size() const noexcept
    {
      return size_bytes() / sizeof(char_type);
    }

    constexpr size_type length() const noexcept
    {
      return length_bytes() / sizeof(char_type);
    }

    constexpr bool empty() const noexcept
    {
      return data() != nullptr && size_bytes() != 0;
    }

    size_type capacity() const noexcept
    {
      return this->MaximumLength / sizeof(char_type);
    }
#pragma endregion

#pragma region Operations
    constexpr size_type copy(pointer dest, size_type count, size_type pos = 0) const
    {
    }

    constexpr basic_string substr(size_type pos = 0, size_type count = npos) const
    {
    }

    constexpr int compare(basic_string s) const noexcept;
    constexpr int compare(size_type pos1, size_type count1, basic_string s) const;
    constexpr int compare(size_type pos1, size_type count1, basic_string s, size_type pos2, size_type count2) const;
    constexpr int compare(const_pointer s) const;
    constexpr int compare(size_type pos1, size_type count1, const_pointer s) const;
    constexpr int compare(size_type pos1, size_type count1, const_pointer s, size_type count2) const;

    constexpr bool equals(const basic_string &other) const
    {
      size_t n1 = size_bytes();
      size_t n2 = other.size_bytes();

      if ( n1 == n2 ) {
        auto s1 = begin();
        auto s2 = other.begin();

        while ( n1 >= sizeof(uintptr_t) ) {
          if ( *(uintptr_t *)s1 != *(uintptr_t *)s2 )
            break;

          n1 -= sizeof(uintptr_t);
          if ( n1 == 0 )
            return true;

          s1 += sizeof(uintptr_t) / sizeof(char_type);
          s2 += sizeof(uintptr_t) / sizeof(char_type);
        }
        while ( s1 < end() ) {
          auto c1 = *s1;
          auto c2 = *s2;
          if ( c1 != c2 )
            return false;

          s1 += 1;
          s2 += 1;
        }
        return true;
      }
      return false;
    }

    constexpr bool iequals(const basic_string &other) const
    {
      const auto &facet = std::use_facet<std::ctype<char_type>>(std::locale());

      size_t n1 = size_bytes();

      if ( n1 == other.size_bytes() ) {
        auto s1 = begin();
        auto s2 = other.begin();

        while ( n1 >= sizeof(uintptr_t) ) {
          if ( *(uintptr_t *)s1 != *(uintptr_t *)s2 )
            break;

          n1 -= sizeof(uintptr_t);
          if ( n1 == 0 )
            return true;

          s1 += sizeof(uintptr_t) / sizeof(char_type);
          s2 += sizeof(uintptr_t) / sizeof(char_type);
        }
        while ( s1 < end() ) {
          const auto c1 = *s1;
          const auto c2 = *s2;
          if ( (c1 != c2) && (facet.tolower(c1) != facet.tolower(c2)) )
            return false;

          s1++;
          s2++;
        }
        return true;
      }
      return false;
    }

    constexpr bool istarts_with(const basic_string &other) const noexcept
    {
      const auto &facet = std::use_facet<std::ctype<char_type>>(std::locale());

      auto s1 = begin();
      auto s2 = other.begin();

      if ( size_bytes() < other.size_bytes() )
        return false;

      while ( s2 < other.end() ) {
        const auto c1 = *s1;
        const auto c2 = *s2;
        if ( (c1 != c2) && (facet.tolower(c1) != facet.tolower(c2)) )
          return false;

        s2++;
        s1++;
      }

      return true;
    }

    constexpr bool starts_with(const basic_string &other) const noexcept
    {
      auto s1 = begin();
      auto s2 = other.begin();

      if ( size_bytes() < other.size_bytes() )
        return false;

      while ( s2 < other.end() ) {
        if ( *s2 != *s1 )
          return false;

        s2++;
        s1++;
      }
      return true;
    }

    constexpr bool ends_with(const basic_string &other) const noexcept
    {
      auto s1 = rbegin();
      auto s2 = other.rbegin();

      if ( size_bytes() < other.size_bytes() )
        return false;

      while ( s2 < other.rend() ) {
        if ( *s2 != *s1 )
          return false;

        s2++;
        s1++;
      }
      return true;
    }

    //constexpr size_type find(basic_string_span v, size_type pos = 0) const noexcept
    //{
    //  if ( v.size() > size() || pos > size() - v.size() )
    //    return static_cast<size_t>(-1);

    //  if ( v.size() == 0 )
    //    return pos;

    //  const auto _Possible_matches_end = _Haystack + (_Hay_size - _Needle_size) + 1;
    //  for ( auto _Match_try = _Haystack + _Start_at;; ++_Match_try ) {
    //    _Match_try = _Traits::find(_Match_try, static_cast<size_t>(_Possible_matches_end - _Match_try), *_Needle);
    //    if ( !_Match_try ) { // didn't find first character; report failure
    //      return static_cast<size_t>(-1);
    //    }

    //    if ( _Traits::compare(_Match_try, _Needle, _Needle_size) == 0 ) { // found match
    //      return static_cast<size_t>(_Match_try - _Haystack);
    //    }
    //  }
    //}
    //constexpr size_type find(value_type ch, size_type pos = 0) const noexcept;
    //constexpr size_type find(const_pointer s, size_type pos, size_type count) const;
    //constexpr size_type find(const_pointer s, size_type pos = 0) const;

    //constexpr size_type rfind(basic_string_span v, size_type pos = npos) const noexcept;
    //constexpr size_type rfind(value_type c, size_type pos = npos) const noexcept;
    //constexpr size_type rfind(const_pointer s, size_type pos, size_type count) const;
    //constexpr size_type rfind(const_pointer s, size_type pos = npos) const;

    //constexpr size_type find_first_of(basic_string_span v, size_type pos = 0) const noexcept;
    //constexpr size_type find_first_of(value_type c, size_type pos = 0) const noexcept;
    //constexpr size_type find_first_of(const_pointer s, size_type pos, size_type count) const;
    //constexpr size_type find_first_of(const_pointer s, size_type pos = 0) const;

    //constexpr size_type find_last_of(basic_string_span v, size_type pos = npos) const noexcept;
    //constexpr size_type find_last_of(value_type c, size_type pos = npos) const noexcept;
    //constexpr size_type find_last_of(const_pointer s, size_type pos, size_type count) const;
    //constexpr size_type find_last_of(const_pointer s, size_type pos = npos) const;

    //constexpr size_type find_first_not_of(basic_string_span v, size_type pos = 0) const noexcept;
    //constexpr size_type find_first_not_of(value_type c, size_type pos = 0) const noexcept;
    //constexpr size_type find_first_not_of(const_pointer s, size_type pos, size_type count) const;
    //constexpr size_type find_first_not_of(const_pointer s, size_type pos = 0) const;

    //constexpr size_type find_last_not_of(basic_string_span v, size_type pos = npos) const noexcept;
    //constexpr size_type find_last_not_of(value_type c, size_type pos = npos) const noexcept;
    //constexpr size_type find_last_not_of(const CharT *s, size_type pos, size_type count) const;
    //constexpr size_type find_last_not_of(const CharT *s, size_type pos = npos) const;
#pragma endregion

    bool operator==(const basic_string &rhs) const
    {
      return this->equals(rhs);
    }

    constexpr operator const T *() const noexcept
    {
      return this;
    }

    constexpr operator T *() noexcept
    {
      return this;
    }

    explicit operator std::string_view() const
    {
      return { data(), size() };
    }
  };

  using ustring = basic_string<UNICODE_STRING>;
  using string = basic_string<ANSI_STRING>;
}
