#include "framework.h"
#include "Hooks.h"

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();
    FILE* File = nullptr;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    MH_Initialize();
    Sleep(5000);

    *(bool*)(ImageBase + Addresses::GIsClient) = false;
    *(bool*)(ImageBase + Addresses::GIsServer) = true;
    Hook(ImageBase + Addresses::GetNetMode, ReturnTrue);
    Hook(ImageBase + Addresses::ReadyToStartMatch, ReadyToStartMatch);
    Hook(ImageBase + Addresses::SpawnDefaultPawnFor, SpawnDefaultPawnFor);
    Hook(ImageBase + Addresses::ActorNetMode, ReturnTrue);
    Hook(ImageBase + Addresses::KickPlayer, ReturnTrue);
    Hook(ImageBase + Addresses::ChangeGameSessionId, ReturnHook);
    Hook(ImageBase + Addresses::TickFlush, TickFlushHook, (void**)&TickFlush);

    UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"open Artemis_Terrain", nullptr);
    UWorld::GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(0, 0, Main, 0, 0, 0);
        break;
    }
    return TRUE;
}