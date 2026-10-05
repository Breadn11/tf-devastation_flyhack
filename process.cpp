#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcessIdByName(const wchar_t* processName)
{
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hProcessSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD processId = 0;

	PROCESSENTRY32 processEntry{};
	processEntry.dwSize = sizeof(processEntry);

	BOOL hasEntry = Process32First(hProcessSnapshot, &processEntry);

	while (hasEntry)
	{
		if (_wcsicmp(processEntry.szExeFile, processName) == 0)
		{
			processId = processEntry.th32ProcessID;
			break;
		}
		hasEntry = Process32Next(hProcessSnapshot, &processEntry);
	}

	CloseHandle(hProcessSnapshot);
	return processId;
}

DWORD GetModuleBaseAddress(DWORD processId, const wchar_t* processName)
{
	HANDLE hProcessSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, processId);

	if (hProcessSnapshot == INVALID_HANDLE_VALUE)
	{
		return 0;
	}

	DWORD moduleBaseAddr = 0;

	MODULEENTRY32 moduleEntry{};
	moduleEntry.dwSize = sizeof(moduleEntry);

	BOOL hasEntry = Module32First(hProcessSnapshot, &moduleEntry);

	while (hasEntry)
	{
		if (_wcsicmp(moduleEntry.szModule, processName) == 0)
		{
			moduleBaseAddr = (DWORD)moduleEntry.modBaseAddr;
			break;
		}
		hasEntry = Module32Next(hProcessSnapshot, &moduleEntry);
	}

	CloseHandle(hProcessSnapshot);
	return moduleBaseAddr;
}