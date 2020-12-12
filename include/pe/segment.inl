#pragma once
#include <ntdll.h>
#include <gsl/span>
#include <gsl/span_ext>
#include "segment.h"
#include "module.h"

namespace pe
{
  inline const class module *segment::module() const
  {
    return get_module_from_address(this);
  }

  inline class module *segment::module()
  {
    return get_module_from_address(this);
  }

  inline std::string_view segment::name() const
  {
    size_t count;
    for ( count = 0; count < IMAGE_SIZEOF_SHORT_NAME; ++count ) {
      if ( !this->Name[count] )
        break;
    }
    return { reinterpret_cast<const char *>(this->Name), count };
  }

  inline gsl::span<uint8_t> segment::as_bytes()
  {
    if ( auto module = this->module() ) {
      return gsl::make_span(reinterpret_cast<uint8_t *>(
        reinterpret_cast<uintptr_t>(module) + this->VirtualAddress), this->Misc.VirtualSize);
    }
    return {};
  }

  inline gsl::span<const uint8_t> segment::as_bytes() const
  {
    if ( auto module = this->module() ) {
      return gsl::make_span(reinterpret_cast<const uint8_t *>(
        reinterpret_cast<uintptr_t>(module) + this->VirtualAddress), this->Misc.VirtualSize);
    }
    return {};
  }

  inline bool segment::contains_code() const
  {
    return this->Characteristics & IMAGE_SCN_CNT_CODE;
  }

  inline bool segment::contains_initialized_data() const
  {
    return this->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA;
  }

  inline bool segment::contains_uninitialized_data() const
  {
    return this->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA;
  }

  inline uint32_t segment::relocation_count() const
  {
    if ( (this->Characteristics & IMAGE_SCN_LNK_NRELOC_OVFL)
      && this->NumberOfRelocations == 0xFFFF ) {

      return reinterpret_cast<const IMAGE_RELOCATION *>(
        reinterpret_cast<uintptr_t>(this->module()) + this->PointerToRelocations)->RelocCount;
    }
    return this->NumberOfRelocations;
  }

  inline bool segment::discardable() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_DISCARDABLE;
  }

  inline bool segment::not_cached() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_NOT_CACHED;
  }

  inline bool segment::not_paged() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_NOT_PAGED;
  }

  inline bool segment::shared() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_SHARED;
  }

  inline bool segment::executable() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_EXECUTE;
  }

  inline bool segment::readable() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_READ;
  }

  inline bool segment::writable() const
  {
    return this->Characteristics & IMAGE_SCN_MEM_WRITE;
  }
}
