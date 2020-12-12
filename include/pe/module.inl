#pragma once
#include <ntapi\string.h>
#include <ntapi\critsec.h>
#include <ntdll.h>
#include <string>
#include <mutex>
#include "module.h"
#include "segment.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace pe
{
  inline uintptr_t module::handle() const
  {
    return reinterpret_cast<uintptr_t>(this);
  }

  inline std::wstring module::base_name() const
  {
    ntapi::critsec crit(NtCurrentPeb()->LoaderLock);
    std::lock_guard<ntapi::critsec> guard(crit);

    auto const Head = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    for ( auto Entry = Head->Flink; Entry != Head; Entry = Entry->Flink ) {
      auto Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

      if ( Module->DllBase == this )
          return std::wstring(Module->BaseDllName.Buffer, Module->BaseDllName.Length / sizeof(wchar_t));
    }
    return {};
  }

  inline std::wstring module::full_name() const
  {
    ntapi::critsec crit(NtCurrentPeb()->LoaderLock);
    std::lock_guard<ntapi::critsec> guard(crit);

    const auto Head = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
    for ( auto Entry = Head->Flink; Entry != Head; Entry = Entry->Flink ) {
      auto Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

      if ( Module->DllBase == this )
        return std::wstring(Module->FullDllName.Buffer, Module->FullDllName.Length / sizeof(wchar_t));
    }
    return {};
  }

  inline IMAGE_DOS_HEADER *module::dos_header()
  {
    return reinterpret_cast<IMAGE_DOS_HEADER *>(this);
  }

  inline const IMAGE_DOS_HEADER *module::dos_header() const
  {
    return reinterpret_cast<const IMAGE_DOS_HEADER *>(this);
  }

  inline IMAGE_NT_HEADERS *module::nt_header()
  {
    const auto dosheader = this->dos_header();
    if ( dosheader->e_magic != IMAGE_DOS_SIGNATURE )
      return nullptr;

    const auto ntheader = this->rva_to<IMAGE_NT_HEADERS>(dosheader->e_lfanew);
    if ( ntheader->Signature != IMAGE_NT_SIGNATURE )
      return nullptr;

    return ntheader;
  }

  inline const IMAGE_NT_HEADERS *module::nt_header() const
  {
    return const_cast<module *>(this)->nt_header();
  }

  inline size_t module::size() const
  {
    const auto dosheader = this->dos_header();
    if ( dosheader->e_magic != IMAGE_DOS_SIGNATURE )
      return 0;

    const auto ntheader = this->nt_header();
    if ( !ntheader
      || !ntheader->FileHeader.SizeOfOptionalHeader
      || ntheader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC )
      return 0;

    return ntheader->OptionalHeader.SizeOfImage;
  }

  inline gsl::span<segment> module::segments()
  {
    const auto ntheader = this->nt_header();
    if ( !ntheader )
      return {};

    return gsl::make_span(
      static_cast<class segment *>(IMAGE_FIRST_SECTION(ntheader)),
      ntheader->FileHeader.NumberOfSections);
  }

  inline gsl::span<const class segment> module::segments() const
  {
    return const_cast<module *>(this)->segments();
  }

  inline class segment *module::segment(const char *name)
  {
    const auto segments = this->segments();
    const auto &it = std::find_if(segments.begin(), segments.end(), [&](const class segment &x) {
      return !x.name().compare(name);
    });
    return it != segments.end() ? &*it : nullptr;
  }

  inline const class segment *module::segment(const char* name) const
  {
    return const_cast<module *>(this)->segment(name);
  }

  inline class export_directory *module::export_directory()
  {
    const auto ntheader = this->nt_header();
    if ( !ntheader
      || !ntheader->FileHeader.SizeOfOptionalHeader
      || ntheader->OptionalHeader.Magic != IMAGE_NT_OPTIONAL_HDR_MAGIC
      || !ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
      || !ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size )
      return nullptr;

    return this->rva_to<class export_directory>(
      ntheader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
  }

  inline const class export_directory *module::export_directory() const
  {
    return const_cast<module *>(this)->export_directory();
  }

  inline void *module::find_function(const char *name) const
  {
    if ( !name ) return nullptr;

    if ( PVOID ProcedureAddress;
      NT_SUCCESS(LdrGetProcedureAddress(const_cast<module *>(this), ntapi::string(name), 0, &ProcedureAddress)) ) {
      return ProcedureAddress;
    }
    return nullptr;
  }

  inline void *module::find_function(uint32_t num) const
  {
    if ( !num ) return nullptr;

    if ( PVOID ProcedureAddress;
      NT_SUCCESS(LdrGetProcedureAddress(const_cast<module *>(this), nullptr, num, &ProcedureAddress)) ) {
      return ProcedureAddress;
    }
    return nullptr;
  };

  inline module *get_module(wchar_t const *name)
  {
    ntapi::critsec crit(NtCurrentPeb()->LoaderLock);

    if ( !name ) {
      return static_cast<module *>(NtCurrentPeb()->ImageBaseAddress);
    } else {
      std::lock_guard<ntapi::critsec> guard(crit);

      auto const Head = &NtCurrentPeb()->Ldr->InLoadOrderModuleList;
      for ( auto Entry = Head->Flink; Entry != Head; Entry = Entry->Flink ) {
        auto Module = CONTAINING_RECORD(Entry, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks);
        if ( !Module->InMemoryOrderLinks.Flink )
          continue;

        if ( static_cast<ntapi::ustring *>(&Module->BaseDllName)->iequals(name) ) {
          return static_cast<module *>(Module->DllBase);
        }
      }
    }
    return nullptr;
  }

  inline module *get_module_from_address(void *pc)
  {
    void *Unused;
    return static_cast<module *>(RtlPcToFileHeader(pc, &Unused));
  }

  inline const module *get_module_from_address(const void *pc)
  {
    return get_module_from_address(const_cast<void *>(pc));
  }

  inline module *instance_module()
  {
    return reinterpret_cast<module *>(&__ImageBase);
  }
}
