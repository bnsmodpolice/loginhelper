#pragma once
#include <ntdll.h>

namespace ntapi
{
  class mprotect
  {
    PVOID _addr;
    SIZE_T _len;
    ULONG _prot;
    NTSTATUS _st;

  public:
    mprotect(PVOID addr, SIZE_T len, ULONG prot)
    {
      _addr = addr;
      _len = len;
      _prot = prot;
      _st = NtProtectVirtualMemory(NtCurrentProcess(), &_addr, &_len, _prot, &_prot);
    }

    ~mprotect()
    {
      NtProtectVirtualMemory(NtCurrentProcess(), &_addr, &_len, _prot, &_prot);
    }

    operator bool() const
    {
      return NT_SUCCESS(_st);
    }
  };
}
