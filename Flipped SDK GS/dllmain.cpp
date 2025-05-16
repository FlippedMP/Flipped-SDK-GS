#include "Other/Framework.h"
#include "Core/Hooks.h"

DWORD WINAPI Main(LPVOID)
{
    AllocConsole();
    FILE* File = nullptr;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SET_TITLE("Flipped 19.10 - Initializing...");
    Sleep(5000);

    *(bool*)(Addresses::GIsClient) = false;
    *(bool*)(Addresses::GIsServer) = true;

    Util::FHookBase::Initialize();

#pragma region Misc
    Util::FHook("AGameSession::KickPlayer", Addresses::GameSessionKickPlayer, ReturnTrue);
    Util::FHook("UNetDriver::TickFlush", Addresses::TickFlush, TickFlush, DEFINE_OG(TickFlushOG));
    Util::FHook("DispatchRequest", Addresses::DispatchRequest, DispatchRequest, DEFINE_OG(DispatchRequestOG));

    Util::FHook("UWorld::GetNetMode", Addresses::WorldGetNetMode, ReturnTrue);
    Util::FHook("AActor::GetNetMode", Addresses::ActorGetNetMode, ReturnTrue);

    int NullCount = 0; // i refuse to use c style loops for this nigga
    for (uint64_t Address : Addresses::NullFunctions)
    {
        NullCount++;
        Util::FHook("Null Function " + std::to_string(NullCount), Address, ReturnHook);
    }
#pragma endregion

#pragma region FortGameModeAthena
    Util::FHook("AFortGameModeAthena::ReadyToStartMatch", Addresses::ReadyToStartMatch, ReadyToStartMatch);
    Util::FHook("AFortGameModeAthena::SpawnDefaultPawnFor", Addresses::SpawnDefaultPawnFor, SpawnDefaultPawnFor);
#pragma endregion

#pragma region FortPlayerControllerAthena
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerExecuteInventoryItem", Addresses::ServerExecuteInventoryItemVFT, ServerExecuteInventoryItem);
    Util::FHook("AFortPlayerControllerAthena::ServerAcknowledgePossession", Addresses::ServerAcknowledgePossession, ServerAcknowledgePossession);
#pragma endregion

    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"open Artemis_Terrain", nullptr);
    GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
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