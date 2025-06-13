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

    Util::FHookBase::Initialize(); // i moved the imagebase log in here!

#pragma region GameSessionPatches
    Util::FHook("UFortGameInstance::GetServerAnalyticsProvider-PatchFix", Addresses::GameSessionPatch, uint8_t(0x85));
    Util::FHook("UnknownPatch - 1", 0x10268A1, uint8_t(0x85)); // adam name this if u remember what it is!
#pragma endregion

#pragma region CommandLine
    if (bUsesGameSessions)
    {
        Util::FHook("FCommandLine::Get", Addresses::FCommandLineGetCommandLine, FCommandLine_GetCommandLine, DEFINE_OG(GetCommandLineOG));
    }
#pragma endregion

#pragma region Misc
    Util::FHook("AGameSession::KickPlayer", Addresses::GameSessionKickPlayer, ReturnTrue);
    Util::FHook("UNetDriver::TickFlush", Addresses::TickFlush, TickFlush, DEFINE_OG(TickFlushOG));
    Util::FHook("DispatchRequest", Addresses::DispatchRequest, DispatchRequest, DEFINE_OG(DispatchRequestOG));

    Util::FHook("UWorld::GetNetMode", Addresses::WorldGetNetMode, ReturnTrue);
    Util::FHook("AActor::GetNetMode", Addresses::ActorGetNetMode, ReturnTrue);
    Util::FHook("UAthenaNavSystemConfig::CreateAndConfigureNavigationSystem", Addresses::CreateAndConfigureNavigationSystem, CreateAndConfigureNavSystem, DEFINE_OG(CreateAndConfigureNavSystemOG));

	Util::FHook<UObject>("UObject::CanCreateInCurrentConext", uint32(0x100 / 8), ReturnTrue);

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
    Util::FHook("AFortGameModeAthena::StartNewSafeZonePhase", Addresses::StartNewSafeZonePhase, StartNewSafeZonePhase, DEFINE_OG(StartNewSafeZonePhaseOG));

    if (bUsesGameSessions) {
        Util::FHook<AFortGameModeAthena>("AFortGameModeAthena::GetGameSessionClass", Addresses::GetGameSessionClassVFT, GetGameSessionClass);
    }

    if (bLategame) {
        Util::FHook("AFortGameModeAthena::OnAircraftEnteredDropZone", Addresses::OnAircraftEnteredDropZone, OnAircraftEnteredDropZone, DEFINE_OG(OnAircraftEnteredDropZoneOG));
        Util::FHook("AFortGameModeAthena::OnAircraftExitedDropZone", Addresses::OnAircraftExitedDropZone, OnAircraftExitedDropZone, DEFINE_OG(OnAircraftExitedDropZoneOG));
    }
#pragma endregion

#pragma region FortPlayerControllerAthena
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerExecuteInventoryItem", Addresses::ServerExecuteInventoryItemVFT, ServerExecuteInventoryItem);
    Util::FHook("AFortPlayerControllerAthena::ServerAcknowledgePossession", Addresses::ServerAcknowledgePossession, ServerAcknowledgePossession);
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerLoadingScreenDropped", Addresses::ServerLoadingScreenDroppedVFT, ServerLoadingScreenDropped);

    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerCreateBuildingActor", Addresses::ServerCreateBuildingActorVFT, ServerCreateBuildingActor, DEFINE_OG(ServerCreateBuildingActorOG));
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerEditBuildingActor", Addresses::ServerEditBuildingActorVFT, ServerEditBuildingActor, DEFINE_OG(ServerEditBuildingActorOG));
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerRepairBuildingActor", Addresses::ServerRepairBuildingActorVFT, ServerRepairBuildingActor, DEFINE_OG(ServerRepairBuildingActorOG));
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerBeginEditingBuildingActor", Addresses::ServerBeginEditingBuildingActorVFT, ServerBeginEditingBuildingActor, DEFINE_OG(ServerBeginEditingBuildingActorOG));
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerEndEditingBuildingActor", Addresses::ServerEndEditingBuildingActorVFT, ServerEndEditingBuildingActor, DEFINE_OG(ServerEndEditingBuildingActorOG));
#pragma endregion

#pragma region AbilitySystemComponent
    Util::FHook<UAbilitySystemComponent>("UAbilitySystemComponent::ServerTryActivateAbilityWithEventData", Addresses::ServerTryActivateAbilityWithEventDataVFT, ServerTryActivateAbilityWithEventData);
    Util::FHook<UFortAbilitySystemComponent>("UFortAbilitySystemComponent::ServerTryActivateAbilityWithEventData", Addresses::ServerTryActivateAbilityWithEventDataVFT, ServerTryActivateAbilityWithEventData);
    Util::FHook<UFortAbilitySystemComponentAthena>("UFortAbilitySystemComponentAthena::ServerTryActivateAbilityWithEventData", Addresses::ServerTryActivateAbilityWithEventDataVFT, ServerTryActivateAbilityWithEventData);
#pragma endregion

#pragma region FortAthenaMutator_GiveItemsAtGamePhaseStep
    Util::FHook("AFortAthenaMutator_GiveItemsAtGamePhaseStep::OnGamePhaseStepChanged", Addresses::OnGamePhaseStepChanged, execOnGamePhaseStepChanged);
#pragma endregion

#pragma region AthenaAIServicePlayerBots
    /*
        MH_CreateHook(LPVOID(Addresses::ImageBase + Addresses::SpawnAI), execSpawnAI, nullptr);
        MH_EnableHook(LPVOID(Addresses::ImageBase + Addresses::SpawnAI));
    */

    Util::FHook("UnknownPatch - 2", 0x5EE0507, uint32_t(0x1C4C899)); // 0x7B2CDA4 - (0x5EE0504 + 7)
    MH_CreateHook(LPVOID(Addresses::ImageBase + 0x7B2CDA4), InitalizeMMRInfos, nullptr);
    MH_EnableHook(LPVOID(Addresses::ImageBase + 0x7B2CDA4));

    // Util::FHook("UAthenaAIServicePlayerBots::WaitForMatchAssignmentReady", uint64(0x5EE9524), WaitForMatchAssignmentReady, DEFINE_OG(WaitForMatchAssignmentReadyOG));
#pragma endregion

#pragma region BuildingContainer
    Util::FHook("ABuldingContainer::SpawnLoot", Addresses::SpawnLoot, SpawnLoot);
#pragma endregion

#pragma region ServiceConfigMcp
    if (bUsesGameSessions) {
        Util::FHook("FServiceConfigMcp::GetServicePermissionsByName", Addresses::ServicePermissionsByName, GetServicePermissionsByName);
    }
#pragma endregion

#pragma region FortControllerComponent_Aircraft
    Util::FHook<UFortControllerComponent_Aircraft>("UFortControllerComponent_Aircraft::ServerAttemptAircraftJump", Addresses::ServerAttemptAircraftJumpVFT, ServerAttemptAircraftJump);
#pragma endregion

    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"open Artemis_Terrain", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogQos", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortInventory all", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortWorld", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortMutatorInventoryOverride all", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFort all", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortInvServiceComp all", nullptr);
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortPlayerRegistration all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogStreaming");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineGame all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAthenaAIServiceBots all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogStats all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAbilitySystem");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogHotfixManager all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineSession all");
    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogMatchmakingServiceDedicatedServer all");

    GetWorld()->OwningGameInstance->LocalPlayers.Remove(0);
    return 0;
}

BOOL APIENTRY DllMain(HMODULE, DWORD Reason, LPVOID)
{
    switch (Reason)
    {
        case DLL_PROCESS_ATTACH: CreateThread(0, 0, Main, 0, 0, 0); break;
    }

    return TRUE;
}