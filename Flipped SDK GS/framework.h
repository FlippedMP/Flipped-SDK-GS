#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include "SDK/SDK.hpp"
using namespace SDK;
#include "MinHook.h"
#include "Addresses.h"
#include "Funcs.h"

static void Hook_Internal(uint64_t Address, PVOID Hook, void** OG = nullptr)
{
	MH_CreateHook(PVOID(Address), Hook, OG);
	MH_EnableHook(PVOID(Address));
}
#define Hook(...) Hook_Internal(__VA_ARGS__);

static int ReturnTrue()
{
	return true;
}

static void ReturnHook()
{
	return;
}