#pragma once
#include <ntdll.h>
#include <cstdint>

namespace ntapi
{
  class critsec
  {
  public:
    using native_handle_type = RTL_CRITICAL_SECTION *;

  private:
    native_handle_type handle;

  public:
    critsec(RTL_CRITICAL_SECTION *crit)
    {
      this->handle = crit;
    }

    void lock()
    {
      RtlEnterCriticalSection(this->handle);
    }

    bool try_lock()
    {
      return RtlTryEnterCriticalSection(this->handle);
    }

    void unlock()
    {
      RtlLeaveCriticalSection(this->handle);
    }

    native_handle_type native_handle()
    {
      return this->handle;
    }
  };
}
