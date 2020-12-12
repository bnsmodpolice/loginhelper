#pragma once
#include <Windows.h>

EXTERN_C int GetModuleVersionInfo(HMODULE hModule, PCWSTR pwszSubBlock, LPCVOID *ppv);
