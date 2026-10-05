#pragma once

#include <Windows.h>

DWORD GetProcessIdByName(const wchar_t* processName);

DWORD GetModuleBaseAddress(DWORD processId, const wchar_t* processName);