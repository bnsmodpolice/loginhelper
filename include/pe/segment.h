#pragma once
#include <ntdll.h>
#include <gsl/span>
#include <gsl/span_ext>
#include "module.h"

namespace pe
{
  class module;

  class segment : public IMAGE_SECTION_HEADER
  {
  public:
    segment() = delete;

    const class module *module() const;
    class module *module();
    std::string_view name() const;
    gsl::span<uint8_t> as_bytes();
    gsl::span<const uint8_t> as_bytes() const;
    bool contains_code() const;
    bool contains_initialized_data() const;
    bool contains_uninitialized_data() const;
    uint32_t relocation_count() const;
    bool discardable() const;
    bool not_cached() const;
    bool not_paged() const;
    bool shared() const;
    bool executable() const;
    bool readable() const;
    bool writable() const;
  };
}

#include "segment.inl"
