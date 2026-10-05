#include <iostream>
#include <Windows.h>
#include "process.h"

namespace config
{
	constexpr const wchar_t* PROCESS_NAME = L"transformersdevastation.exe";
	const float MOVE_DISTANCE = 1.0f;
	const int   SLEEP_TIME_S  = 4;
	const char  KEY_FLY = 'F';
}

namespace instr
{
	const DWORD NOP        = 0x90909090;
	const DWORD OG_GRAVITY = 0x5056290F;
	const DWORD OG_STICK   = 0x5057290F;
}

namespace offset
{
	const DWORD IN_GRAVITY = 0x54136;
	const DWORD IN_STICK   = 0x54B23;
	const DWORD PLAYER     = 0xA5CE84;
	const DWORD CAMERA     = 0xB73F68;
}

namespace addr
{
	DWORD player{};
	DWORD playerX{};
	DWORD playerZ{};
	DWORD playerY{};
	DWORD camera{};
	DWORD cameraPitch{};
	DWORD cameraYaw{};
}

namespace val
{
	float playerX{};
	float playerZ{};
	float playerY{};
	float newPlayerX{};
	float newPlayerZ{};
	float newPlayerY{};
	float cameraPitch{};
	float cameraYaw{};
}

namespace proc
{
	DWORD  pID{};
	DWORD  addrBase{};
	HANDLE handle{};
}

namespace state
{
	bool isFlyToggled{};
	bool isKeyDown_Fly{};
	bool wasKeyDown_Fly{};
}

template <typename T>
bool MemRead(DWORD addr, T* output)
{
	return ReadProcessMemory(proc::handle, (LPCVOID)(addr), output, 4, NULL);
}

template <typename T>
bool MemWrite(DWORD addr, T* input)
{
	return WriteProcessMemory(proc::handle, (LPVOID)(addr), input, 4, NULL);
}

void ResolveProcessDetails()
{
	proc::pID = GetProcessIdByName(config::PROCESS_NAME);
	proc::addrBase = GetModuleBaseAddress(proc::pID, config::PROCESS_NAME);
	proc::handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, proc::pID);
}

void ToggleFly()
{
	state::isFlyToggled = !state::isFlyToggled;

	if (state::isFlyToggled)
	{
		MemWrite(proc::addrBase + offset::IN_GRAVITY, &instr::NOP);
		MemWrite(proc::addrBase + offset::IN_STICK, &instr::NOP);
	}
	else
	{
		MemWrite(proc::addrBase + offset::IN_GRAVITY, &instr::OG_GRAVITY);
		MemWrite(proc::addrBase + offset::IN_STICK, &instr::OG_STICK);
	}
}

void MainLoop()
{
	state::isKeyDown_Fly = GetAsyncKeyState(config::KEY_FLY);

	if (state::isKeyDown_Fly && !state::wasKeyDown_Fly)
	{
		ResolveProcessDetails();
		ToggleFly();
	}
	
	state::wasKeyDown_Fly = state::isKeyDown_Fly;
}

int main()
{
	while (true)
	{
		MainLoop();
		
		Sleep(config::SLEEP_TIME_S);
	}
}