#pragma once
#include <ntdll.h>
#include "export_directory.h"
#include "module.h"

namespace pe
{
  inline const module *export_directory::module() const
  {
    return get_module_from_address(this);
  }

  inline module *export_directory::module()
  {
    return get_module_from_address(this);
  }

  inline __time32_t export_directory::timestamp() const
  {
    return this->TimeDateStamp;
  }

  inline const char *export_directory::name() const
  {
    return this->module()->rva_to<const char>(this->Name);
  }
}
