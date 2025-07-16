#include "Other/Framework.h"
#include "Core/Hooks.h"


DWORD WINAPI Main(LPVOID)
{
    AllocConsole();
    FILE* File = nullptr;
    freopen_s(&File, "CONOUT$", "w+", stdout);
    SET_TITLE("Flipped 19.10 - Initializing...");
    Sleep(5000);

    printf("MM: %p", Addresses::ImageBase);

    *(bool*)(Addresses::GIsClient) = false;
    *(bool*)(Addresses::GIsServer) = true;

    Util::FHookBase::Initialize(); // i moved the imagebase log in here!

#pragma region GameSessionPatches
    Util::FHook("UFortGameInstance::GetServerAnalyticsProvider-PatchFix", Addresses::GameSessionPatch, uint8_t(0x85));
    Util::FHook("UnknownPatch - 1", 0x10268A1, uint8_t(0x85)); // adam name this if u remember what it is!
    Util::FHook("GameSessionPlayerEligibility", 0x64540A0, uint8_t(0xC3));
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

    Util::FHook("CanActivateAbility", uint64_t(0x4dd8528), ReturnTrue);

    //Util::FHook("UObject::ProcessEvent", uint64_t(Offsets::ProcessEvent), ProcessEvent, DEFINE_OG(ProcessEventOG));

    Util::FHook("UFortKismetLibrary::GiveItemToInventoryOwner", uint64_t(0x6b6bb30), execGiveItemToInventoryOwner);

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
    //Util::FHook("AFortGameModeAthena::StartAircraftPhase", uint64_t(0x5FA4538), StartAircraftPhase, DEFINE_OG(StartAircraftPhaseOG));

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
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerGiveCreativeItem", Addresses::ServerGiveCreativeItemVFT, ServerGiveCreativeItem);
    Util::FHook("AFortPlayerControllerAthena::ClientOnPawnDied", uint64_t(0x6C26888), ClientOnPawnDiedHook, DEFINE_OG(ClientOnPawnDiedOG));
    Util::FHook("AFortPlayerControllerAthena::GetPlayerViewpoint", uint64_t(0xEE8834), GetPlayerViewPointHook);
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerPlayEmoteItem", uint32_t(0x1E8), ServerPlayEmoteItem, DEFINE_OG(ServerPlayEmoteItemOG));
    Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerAttemptInventoryDrop", uint32_t(0x23A), ServerAttemptInventoryDrop);
#pragma endregion

#pragma region FortPawn
    Util::FHook("AFortPawn::MovingEmoteStopped", uint64_t(0x1FF3A30), MovingEmoteStopped, DEFINE_OG(MovingEmoteStoppedOG));
    Util::FHook("AFortPlayerPawnAthena::OnCapsuleBeginOverlap", uint64_t(0x115A604), OnCapsuleBeginOverlapHook);
    Util::FHook<AFortPlayerPawnAthena>("AFortPlayerPawn::ServerHandlePickupInfo", uint32_t(0x220), ServerHandlePickupInfo);
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
    
    PatchUse<uint16>(Addresses::ImageBase + 0x5EE9590, uint16_t(0xe990));

    PatchUse<uint32>(Addresses::ImageBase + 0x5EE0507, uint32_t(0x1c4c899));
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

#pragma region FortAthenaCreativePortal
    if (bCreative)
    {
        UFunction* Func = UObject::FindObject<UFunction>("Function FortniteGame.FortAthenaCreativePortal.TeleportPlayerToLinkedVolume");
        UFunction* Func2 = UObject::FindObject<UFunction>("Function Engine.PlayerController.OnServerStartedVisualLogger");
        Func->ExecFunction = Func2->ExecFunction;
        Util::FHook("AFortAthenaCreativePortal::TeleportPlayerToLinkedVolume", (uint64_t(Func2->ExecFunction) - Addresses::ImageBase), TeleportPlayerToLinkedVolume);
    }
#pragma endregion

#pragma region FortAthenaAISpawnerDataComponent_AIBotCosmeticBase
    Util::FHook("UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase::GetDances", uint64_t(0x6A41D5C), AI::GetDances);
    Util::FHook("UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase::GetLoadout", uint64_t(0x6A41DE4), AI::GetLoadout);
    Util::FHook("UFortAthenaAISpawnerDataComponent_InventoryBase::GetInventoryItems", uint64_t(0x6A42F4C), AI::GetInventoryItems);
    Util::FHook("UFortAthenaAISpawnerDataComponent_InventoryBase::OnSpawned", uint64_t(0x5EC8548), AI::PostOnSpawned, DEFINE_OG(AI::PostOnSpawnedOG));
#pragma endregion

#pragma region FortPickup
    Util::FHook<AFortPickupAthena>("AFortPickup::GivePickupTo", uint32_t(0xDA), GivePickupTo, DEFINE_OG(GivePickupToOG));
    IFortInventoryOwnerInterface* InventoryOwner = (IFortInventoryOwnerInterface*)Native::GetInterfaceAddress(AFortPlayerControllerAthena::GetDefaultObj(), IFortInventoryOwnerInterface::StaticClass());
    VirtualHookInternal(InventoryOwner->VTable, 0x2F, RemoveInventoryItem);
#pragma endregion


#pragma region FortQuestManager
#pragma endregion

	Util::FHook("ABuildingActor::OnDamageServer", uint64_t(0x69E3008), OnDamageServer, DEFINE_OG(OnDamageServerOG));

    //Util::FHook<AFortPlayerControllerAthena>("AFortPlayerControllerAthena::ServerClientIsReadyToRespawn", uint32_t(0x553), ServerClientIsReadyToRespawn);

    FString MapName = bCreative ? L"open Creative_NoApollo_Terrain" : L"open Artemis_Terrain";

    UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), MapName, nullptr);
    if (bLog) {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogQos", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortInventory all", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortWorld", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortMutatorInventoryOverride all", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFort Verbose", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFort Verbose", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortInvServiceComp all", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortPlayerRegistration all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogStreaming");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineGame all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAthenaAIServiceBots all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogStats all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogHotfixManager all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineSession all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogMatchmakingServiceDedicatedServer all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogGameplayTags all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogJson all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogOnlineSession all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortDayNight all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogLivingWorldManager all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAI Verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAbility Verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAIPerception Verbose");
        //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log logAthenaBots Verbose");
        //UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log lognavigation verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogCharacter verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogBehaviorTree Verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogPathFollowing Verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogPawnAction verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortAIDirector Verbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortReplicationGraph");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortQuest all");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log logOnline");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogPlayerQuestProgress VeryVerbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortQuestObjective VeryVerbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAISpawner VeryVerbose");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAbilitySystem");
    }
    else {
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log off");
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogFortUIDirector", nullptr);
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"log LogAbilitySystem all", nullptr);
    }


    if (bDisableAI)
        UKismetSystemLibrary::ExecuteConsoleCommand(GetWorld(), L"Phoebe.Enabled 0");

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