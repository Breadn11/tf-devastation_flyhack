#include <iostream>
#include <Windows.h>
#include "process.h"

namespace config
{
	constexpr const wchar_t* PROCESS_NAME = L"transformersdevastation.exe";
	const float MOVE_DISTANCE = 1.0f;
	const int   SLEEP_TIME_MS = 4;
	const float HALF_PI       = 1.5707963f;
}

namespace key
{
	const char TOGGLE  = 'P';
	const char FORWARD = 'I';
	const char LEFT    = 'J';
	const char BACK    = 'K';
	const char RIGHT   = 'L';
	const char UP      = 'U';
	const char DOWN    = 'O';
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
	const DWORD PLAYER_X   = 0x50;
	const DWORD PLAYER_Z   = 0x54;
	const DWORD PLAYER_Y   = 0x58;
	const DWORD CAMERA     = 0xB73F68;
	const DWORD CAM_PITCH  = 0x5E0;
	const DWORD CAM_YAW    = 0x5E4;
}

namespace addr
{
	DWORD player{};
	DWORD camera{};
}

namespace val
{
	float playerX{};
	float playerZ{};
	float playerY{};
	float newPlayerX{};
	float newPlayerZ{};
	float newPlayerY{};
	float addPlayerX{};
	float addPlayerZ{};
	float addPlayerY{};
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
	bool hasUpdated{};

	enum Direction
	{
		Forward,
		Left,
		Back,
		Right,
		Up,
		Down,
	};
	Direction direction;
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

void UpdateAddVector()
{
	if (state::hasUpdated == false)
	{
		val::addPlayerX = 0;
		val::addPlayerY = 0;
		val::addPlayerZ = 0;
		ResolveProcessDetails();
		MemRead(proc::addrBase + offset::CAMERA, &addr::camera);
		MemRead(addr::camera + offset::CAM_PITCH, &val::cameraPitch);
		MemRead(addr::camera + offset::CAM_YAW, &val::cameraYaw);
	}
	
	if (state::direction == state::Forward)
	{
		val::addPlayerX += (std::cos(val::cameraPitch) * std::cos(val::cameraYaw + config::HALF_PI)) * config::MOVE_DISTANCE;
		val::addPlayerZ += std::sin(val::cameraPitch) * config::MOVE_DISTANCE;
		val::addPlayerY += ((std::cos(val::cameraPitch) * std::sin(val::cameraYaw + config::HALF_PI)) * config::MOVE_DISTANCE) * -1;
	}
	if (state::direction == state::Back)
	{
		val::addPlayerX += ((std::cos(val::cameraPitch) * std::cos(val::cameraYaw + config::HALF_PI)) * config::MOVE_DISTANCE) * -1;
		val::addPlayerZ += (std::sin(val::cameraPitch) * config::MOVE_DISTANCE) * -1;
		val::addPlayerY += (std::cos(val::cameraPitch) * std::sin(val::cameraYaw + config::HALF_PI)) * config::MOVE_DISTANCE;
	}
	if (state::direction == state::Left)
	{
		val::addPlayerX += std::cos(val::cameraYaw) * config::MOVE_DISTANCE * -1;
		val::addPlayerY += std::sin(val::cameraYaw) * std::cos(val::cameraPitch) * config::MOVE_DISTANCE;
	}
	if (state::direction == state::Right)
	{
		val::addPlayerX += std::cos(val::cameraYaw) * config::MOVE_DISTANCE;
		val::addPlayerY += std::sin(val::cameraYaw) * std::cos(val::cameraPitch) * config::MOVE_DISTANCE * -1;
	}
	if (state::direction == state::Up)
	{
		val::addPlayerZ += config::MOVE_DISTANCE;
	}
	if (state::direction == state::Down)
	{
		val::addPlayerZ += config::MOVE_DISTANCE * -1;
	}

	state::hasUpdated = true;
}

void MainLoop()
{
	state::isKeyDown_Fly = GetAsyncKeyState(key::TOGGLE);

	if (state::isKeyDown_Fly && !state::wasKeyDown_Fly)
	{
		ResolveProcessDetails();
		ToggleFly();
	}
	
	state::wasKeyDown_Fly = state::isKeyDown_Fly;

	state::hasUpdated = false;

	if (GetAsyncKeyState(key::FORWARD))
	{
		state::direction = state::Forward;
		UpdateAddVector();
	}
	if (GetAsyncKeyState(key::BACK))
	{
		state::direction = state::Back;
		UpdateAddVector();
	}
	if (GetAsyncKeyState(key::LEFT))
	{
		state::direction = state::Left;
		UpdateAddVector();
	}
	if (GetAsyncKeyState(key::RIGHT))
	{
		state::direction = state::Right;
		UpdateAddVector();
	}
	if (GetAsyncKeyState(key::UP))
	{
		state::direction = state::Up;
		UpdateAddVector();
	}
	if (GetAsyncKeyState(key::DOWN))
	{
		state::direction = state::Down;
		UpdateAddVector();
	}

	if (state::hasUpdated == true)
	{
		MemRead(proc::addrBase + offset::PLAYER, &addr::player);
		MemRead(addr::player + offset::PLAYER_X, &val::playerX);
		MemRead(addr::player + offset::PLAYER_Z, &val::playerZ);
		MemRead(addr::player + offset::PLAYER_Y, &val::playerY);
		
		val::newPlayerX = val::playerX + val::addPlayerX;
		val::newPlayerZ = val::playerZ + val::addPlayerZ;
		val::newPlayerY = val::playerY + val::addPlayerY;

		MemWrite(addr::player + offset::PLAYER_X, &val::newPlayerX);
		MemWrite(addr::player + offset::PLAYER_Z, &val::newPlayerZ);
		MemWrite(addr::player + offset::PLAYER_Y, &val::newPlayerY);
	}
}

int main()
{
	while (true)
	{
		MainLoop();
		Sleep(config::SLEEP_TIME_MS);
	}
}