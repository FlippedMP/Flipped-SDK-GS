#pragma once
#include "../Other/Framework.h"
#include "./Inventory.h"
#include"./Looting.h"

void (*DispatchRequestOG)(__int64, __int64, int); void DispatchRequest(__int64 a1, __int64 a2, int a3) { return DispatchRequestOG(a1, a2, 3); }
void (*TickFlushOG)(UNetDriver*); void TickFlush(UNetDriver* Driver) { if (Driver && Driver->ReplicationDriver && Driver->ClientConnections.Num() > 0) Native::ServerReplicateActors(Driver->ReplicationDriver); TickFlushOG(Driver); }
void (*StartNewSafeZonePhaseOG)(AFortGameModeAthena* GameMode, int32_t NewSafeZonePhase);
void (*OnAircraftEnteredDropZoneOG)(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft);
const wchar_t* (*GetCommandLineOG)();
void (*CreateAndConfigureNavSystemOG)(UAthenaNavSystemConfig*, UWorld*);
void (*WaitForMatchAssignmentReadyOG)(UAthenaAIServicePlayerBots*, __int64);
void (*ServerCreateBuildingActorOG)(AFortPlayerControllerAthena* thisPtr, FCreateBuildingActorData BuildingActorData);

static void SetPlaylist(AFortGameModeAthena* GameMode, UFortPlaylistAthena* Playlist) {
	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(GameMode->GameState);

	if (!GameState || !Playlist)
		return;

	GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
	GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
	GameState->CurrentPlaylistInfo.MarkArrayDirty();

	GameState->CurrentPlaylistId = Playlist->PlaylistId;

	GameMode->CurrentPlaylistId = Playlist->PlaylistId;
	GameMode->CurrentPlaylistName = Playlist->PlaylistName;
	GameMode->WarmupRequiredPlayerCount = 1;

	GameState->AirCraftBehavior = Playlist->AirCraftBehavior;
	GameState->CachedSafeZoneStartUp = Playlist->SafeZoneStartUp;
}

bool ReadyToStartMatch(AFortGameModeAthena* thisPtr)
{
	if (!thisPtr)
		return false;

	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(thisPtr->GameState);
	if (!GameState || !GameState->MapInfo)
		return false;

	if (thisPtr->CurrentPlaylistId == -1)
	{
		if (UFortPlaylistAthena* Playlist =
			UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo"))
		{
			SetPlaylist(thisPtr, Playlist);

			thisPtr->AISettings = Playlist->AISettings;
			if (thisPtr->AISettings)
				thisPtr->AISettings->AIServices[1] = UAthenaAIServicePlayerBots::StaticClass();

			for (int i = 0; i < Playlist->AdditionalLevels.Num(); i++)
			{
				TSoftObjectPtr<UWorld> World = Playlist->AdditionalLevels[i];
				FString LevelName = UKismetStringLibrary::Conv_NameToString(World.ObjectID.AssetPathName);
				ULevelStreamingDynamic::LoadLevelInstance(GetWorld(), LevelName, {}, {}, nullptr, FString(), {});
				FAdditionalLevelStreamed NewLevel{ World.ObjectID.AssetPathName,false };
				GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			}

			for (TSoftObjectPtr < UWorld> CurrentAdditionalLevelServerOnly : Playlist->AdditionalLevelsServerOnly)
			{
				FString LevelName = UKismetStringLibrary::Conv_NameToString(CurrentAdditionalLevelServerOnly.ObjectID.AssetPathName);
				ULevelStreamingDynamic::LoadLevelInstance(GetWorld(), LevelName, {}, {}, nullptr, FString(), {});

				FAdditionalLevelStreamed NewLevel{
					CurrentAdditionalLevelServerOnly.ObjectID.AssetPathName,
					true
				};

				GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			}

			GameState->OnFinishedShowingAdditionalPlaylistLevel();
			GameState->OnRep_AdditionalPlaylistLevelsStreamed();
			thisPtr->HandleAllPlaylistLevelsVisible();
			GameState->OnRep_CurrentPlaylistInfo();
			GameState->OnRep_CurrentPlaylistId();

			thisPtr->AIDirector = Misc::SpawnActor<AAthenaAIDirector>();
			thisPtr->AIDirector->Activate();

			if (!thisPtr->AIGoalManager)
				thisPtr->AIGoalManager = Misc::SpawnActor<AFortAIGoalManager>();

			GameState->DefaultRedeployGliderHeightLimit = 10000;
		}
	}

	if (!thisPtr->bWorldIsReady)
	{
		if (UNetDriver* NetDriver = Native::CreateNetDriver_Local(
			GetEngine(),
			Native::GetWorldFromContextObject(GetEngine(), GetWorld()),
			NAME_GameNetDriver)
			)
		{
			GetWorld()->NetDriver = NetDriver;
			GetWorld()->NetDriver->NetDriverName = NAME_GameNetDriver;
			GetWorld()->NetDriver->World = GetWorld();

			FURL InURL;
			InURL.Port = 7777;

			Native::InitListen(GetWorld()->NetDriver, GetWorld(), InURL, false, {});
			Native::SetWorld(GetWorld()->NetDriver, GetWorld());

			for (FLevelCollection& LevelCollection : GetWorld()->LevelCollections)
				LevelCollection.NetDriver = GetWorld()->NetDriver;



			thisPtr->bWorldIsReady = true;

			for (auto& SupportedAthenaLootTierGroup : thisPtr->SupportedAthenaLootTierGroups) {
				FLIPPED_LOG("SupportedAthenaLootTierGroups: " + SupportedAthenaLootTierGroup.ToString());
			}

			for (auto& [SupportTierGroup, Redirect] : thisPtr->RedirectAthenaLootTierGroups) {
				FLIPPED_LOG(SupportTierGroup.ToString());
				FLIPPED_LOG(Redirect.ToString());
			} /* using this we can determine if TierGroup == SupportTierGroup, Redirect to AthenaTierGroup */

			GameState->DefaultRebootMachineHotfix = 1;

			if (auto InventoryManager = Util::Cast<UFortGameInstance>(UWorld::GetWorld()->OwningGameInstance)->InventoryManager) {
				FLIPPED_LOG("Inventory Manager");
			}
			else {
				Util::Cast<UFortGameInstance>(UWorld::GetWorld()->OwningGameInstance)->InventoryManager =
					(UFortInventoryManager*)UGameplayStatics::SpawnObject(UFortInventoryManager::StaticClass(),
						Util::Cast<UFortGameInstance>(UWorld::GetWorld()->OwningGameInstance));
				Util::Cast<UFortGameInstance>(UWorld::GetWorld()->OwningGameInstance)->InventoryManager->PersistenceManager =
					(UVkPersistenceManager*)UGameplayStatics::SpawnObject(UVkPersistenceManager::StaticClass(),
						Util::Cast<UFortGameInstance>(UWorld::GetWorld()->OwningGameInstance)->InventoryManager);
			} /*So Improper*/
			
			TArray<AActor*> WarmupActors;
			static UClass* WarmupClass = Native::StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_Warmup.Tiered_Athena_FloorLoot_Warmup_C");
			UGameplayStatics::GetAllActorsOfClass(GetWorld(), WarmupClass, &WarmupActors);

			for (auto& WarmupActor : WarmupActors)
			{
				auto Container = (ABuildingContainer*)WarmupActor;

				Container->BP_SpawnLoot(nullptr);

				Container->K2_DestroyActor();
			}
			WarmupActors.Free();

			SET_TITLE("Flipped 19.10 - Listening!");
		}

	}

	if (bUsesGameSessions) {
		static bool bWaitedForPlaylist = false;
		if (!bWaitedForPlaylist) {
			if (!thisPtr->CurrentBucketId.IsValid())
				return false;

			FLIPPED_LOG(thisPtr->CurrentBucketId.ToString());
			FLIPPED_LOG("JOBS");
			bWaitedForPlaylist = true;
		}
	}

	static bool bLoadedProperly = false;
	if (!bLoadedProperly)
	{
		for (ULevel*& Level : GetWorld()->Levels)
		{
			if (!Level)
				continue;

			for (AActor*& Actor : Level->Actors)
			{
				if (!Actor)
					continue;

				if (Actor->IsA(AFortPlayerStartWarmup::StaticClass()))
				{
					bLoadedProperly = true;
					break;
				}
			}
		}

		if (!bLoadedProperly)
			return false;
	}

	

	return thisPtr->NumPlayers >= thisPtr->WarmupRequiredPlayerCount;
}

APawn* SpawnDefaultPawnFor(AFortGameModeAthena* thisPtr, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
	if (!thisPtr || !thisPtr->GameState || !NewPlayer || !StartSpot)
		return nullptr;

	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(thisPtr->GameState);
	AFortPlayerPawnAthena* NewPawn = Util::Cast<AFortPlayerPawnAthena>(
		thisPtr->SpawnDefaultPawnAtTransform(NewPlayer, StartSpot->GetTransform())
	);

	if (!NewPawn)
	{
		FLIPPED_LOG("Failed to spawn in new pawn!");
		return nullptr;
	}

	for (FItemAndCount& _ : thisPtr->StartingItems)
	{
		if (!_.Item || _.Count == 0 || _.Item->GetName().contains("Smart"))
			continue;

		Inventory::AddItem(NewPlayer, _.Item, _.Count);
	}

	if (FFortAthenaLoadout* CosmeticLoadoutPC = &NewPlayer->CosmeticLoadoutPC)
	{
		if (CosmeticLoadoutPC->Pickaxe)
			Inventory::AddItem(NewPlayer, CosmeticLoadoutPC->Pickaxe->WeaponDefinition, 1);
		else
			FLIPPED_LOG("Failed to get pickaxe!");
	}

	static bool bFirst = false;
	
	if (!bFirst) {
		UCurveTable* AthenaGameDataTable = Util::Cast<AFortGameStateAthena>(thisPtr->GameState)->AthenaGameDataTable;
		if (AthenaGameDataTable) 
		{
			for (auto& RowPair : AthenaGameDataTable->GetRowMap())
			{
				FName RowName = RowPair.First;
				FSimpleCurve* Curve = reinterpret_cast<FSimpleCurve*>(RowPair.Second);

				if (RowName.ToString() == "Default.SafeZone.Damage") 
				{
					FLIPPED_LOG("Apply SafeZoneDamage");
					for (auto& Key : Curve->Keys)
					{
						FSimpleCurveKey* KeyPtr = &Key;
						if (KeyPtr->Time == 0.f) KeyPtr->Value = 0.f;
					}
				}
			}
		}
		else FLIPPED_LOG("Invalid AthenaGameDataTable!");

		static UDataTable* AthenaRangedWeapons = UObject::FindObject<UDataTable>("DataTable AthenaRangedWeapons.AthenaRangedWeapons");
		Misc::ApplyDataTablePatch(AthenaRangedWeapons);

		bFirst = true;
	}

	return NewPawn;
}


void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32_t OverrideSafeZonePhase)
{
	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(GameMode->GameState);
	if (!GameState)
		return StartNewSafeZonePhaseOG(GameMode, OverrideSafeZonePhase);

	static int LategameSafeZonePhase = 2;

	static UCurveTable* FortGameData = GameState->AthenaGameDataTable;
	static FName ShrinkTimeFName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.ShrinkTime");
	static FName HoldTimeFName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.WaitTime");

	if (FortGameData) {
		for (int i = 0; i < GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached.Num(); i++) {
			UDataTableFunctionLibrary::EvaluateCurveTableRow(FortGameData, ShrinkTimeFName, i, nullptr,
				&GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached[i], {});
		}
		for (int i = 0; i < GameState->MapInfo->SafeZoneDefinition.WaitTimeCached.Num(); i++) {
			UDataTableFunctionLibrary::EvaluateCurveTableRow(FortGameData, HoldTimeFName, i, nullptr,
				&GameState->MapInfo->SafeZoneDefinition.WaitTimeCached[i], {});
		}
		for (int i = 0; i < GameState->MapInfo->SafeZoneDefinition.ForceDistanceMinCached.Num(); i++) {
			UDataTableFunctionLibrary::EvaluateCurveTableRow(GameState->MapInfo->SafeZoneDefinition.ForceDistanceMin.Curve.CurveTable,
				GameState->MapInfo->SafeZoneDefinition.ForceDistanceMin.Curve.RowName, i, nullptr, &GameState->MapInfo->SafeZoneDefinition.ForceDistanceMinCached[i], {});
		}
	}

	if (bLategame) 
	{
		GameMode->SafeZonePhase = LategameSafeZonePhase;
		GameState->SafeZonePhase = LategameSafeZonePhase;
		StartNewSafeZonePhaseOG(GameMode, OverrideSafeZonePhase);
		LategameSafeZonePhase++;
	}
	else StartNewSafeZonePhaseOG(GameMode, OverrideSafeZonePhase);

	float ChosenWaitTime = 0;
	float ShrinkingTime = 0;

	if (GameMode->SafeZonePhase >= 0 && GameMode->SafeZonePhase < GameState->MapInfo->SafeZoneDefinition.WaitTimeCached.Num())
		ChosenWaitTime = GameState->MapInfo->SafeZoneDefinition.WaitTimeCached[GameMode->SafeZonePhase];
	
	if (GameMode->SafeZonePhase >= 0 && GameMode->SafeZonePhase < GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached.Num())
		ShrinkingTime = GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached[GameMode->SafeZonePhase];

	GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = GameState->GetServerWorldTimeSeconds() + ChosenWaitTime;
	GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameState->GetServerWorldTimeSeconds() + ShrinkingTime;

	if (bLategame && 
		(GameMode->SafeZonePhase == 2 || GameMode->SafeZonePhase == 3))
	{
		GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = GameState->GetServerWorldTimeSeconds();
		GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameState->GetServerWorldTimeSeconds() + 0.2;
	}
}

void ServerAcknowledgePossession(AFortPlayerControllerAthena* thisPtr, AFortPlayerPawnAthena* P)
{
	if (!thisPtr || !P)
		return;

	thisPtr->AcknowledgedPawn = P;

	AFortPlayerStateAthena* PlayerState = Util::Cast<AFortPlayerStateAthena>(thisPtr->PlayerState);
	if (!PlayerState)
		return;

	AFortGameModeAthena* GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;

	PlayerState->HeroType = thisPtr->CosmeticLoadoutPC.Character->HeroDefinition;
	UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);

	if (bLategame && GameMode->SafeZonePhase > 2)
	{
		Inventory::AddItem(thisPtr, UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Wood), 500);
	}
}

void ServerLoadingScreenDropped(AFortPlayerControllerAthena* thisPtr) 
{
	AFortPlayerPawnAthena* Pawn = Util::Cast<AFortPlayerPawnAthena>(thisPtr->MyFortPawn);
	if (!Pawn) return;
	void* InterfaceAddress = Native::GetInterfaceAddress(Pawn, IAbilitySystemInterface::StaticClass());

	if (!InterfaceAddress)
		return;

	TScriptInterface<IAbilitySystemInterface> Script;
	Script.ObjectPointer = Pawn;
	Script.InterfacePointer = InterfaceAddress;

	UFortAbilitySet* DefaultAbilitySet = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer");

	if (DefaultAbilitySet)
		UFortKismetLibrary::EquipFortAbilitySet(Script, DefaultAbilitySet, nullptr);
	else
		FLIPPED_LOG("Invalid DefaultAbilitySet!");

	thisPtr->StatManager = (UStatManager*)UGameplayStatics::SpawnObject(UStatManager::StaticClass(), thisPtr);
}

void ServerExecuteInventoryItem(AFortPlayerControllerAthena* thisPtr, const FGuid& ItemGUID)
{
	if (!thisPtr || thisPtr->IsInAircraft())
		return;

	for (FFortItemEntry& ItemEntry : thisPtr->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ItemEntry.ItemDefinition->IsA(UFortTrapItemDefinition::StaticClass())) // just in case | idk if u know how to fix adam but i don't!
			return;

		if (ItemEntry.ItemGuid == ItemGUID)
		{
			if (!thisPtr->MyFortPawn) return;
			thisPtr->MyFortPawn->EquipWeaponDefinition(
				Util::Cast<UFortWeaponItemDefinition>(ItemEntry.ItemDefinition),
				ItemGUID,
				ItemEntry.TrackerGuid,
				false
			);

			return;
		}
	}
}

void ServerTryActivateAbilityWithEventData(UAbilitySystemComponent* thisPtr, FGameplayAbilitySpecHandle AbilityToActivate, bool InputPressed, const FPredictionKey& PredictionKey, const FGameplayEventData* TriggerEventData)
{
	if (!thisPtr)
		return;

	FGameplayAbilitySpec* SelectedAbilitySpec = nullptr;
	for (FGameplayAbilitySpec& AbilitySpec : thisPtr->ActivatableAbilities.Items)
	{
		if (AbilitySpec.Handle.Handle == AbilityToActivate.Handle)
		{
			SelectedAbilitySpec = &AbilitySpec;
			break;
		}
	}

	if (!SelectedAbilitySpec)
	{
		thisPtr->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Current);
		return;
	}

	UGameplayAbility* InstancedAbility = nullptr;
	SelectedAbilitySpec->InputPressed = true;

	if (!Native::InternalTryActivateAbility(thisPtr, AbilityToActivate, PredictionKey, &InstancedAbility, nullptr, TriggerEventData))
	{
		thisPtr->ClientActivateAbilityFailed(AbilityToActivate, PredictionKey.Current);
		SelectedAbilitySpec->InputPressed = false;
		thisPtr->ActivatableAbilities.MarkItemDirty(*SelectedAbilitySpec);
	}
}

void execOnGamePhaseStepChanged(AFortAthenaMutator_GiveItemsAtGamePhaseStep* thisPtr, FFrame* Stack) 
{
	TScriptInterface<IFortSafeZoneInterface> z_Param_Out_SafeZoneInterfaceTemp;
	TScriptInterface<IFortSafeZoneInterface>* p_Z_Param_Out_SafeZoneInterfaceTemp;
	EAthenaGamePhaseStep Z_Param_GamePhaseStep;

	if (Stack->Code())
		Stack->Step(Stack->Object(), &z_Param_Out_SafeZoneInterfaceTemp);
	else
	{
		FProperty* PropertyChainForCompiledIn = Stack->PropertyChainForCompiledIn();
		Stack->PropertyChainForCompiledIn() = (FProperty*)PropertyChainForCompiledIn->Next;
		Stack->StepExplicitProperty((unsigned __int8*)&z_Param_Out_SafeZoneInterfaceTemp, PropertyChainForCompiledIn);
	}

	unsigned __int8* MostRecentPropertyAddress = Stack->MostRecentPropertyAddress();

	p_Z_Param_Out_SafeZoneInterfaceTemp = &z_Param_Out_SafeZoneInterfaceTemp;
	Z_Param_GamePhaseStep = EAthenaGamePhaseStep::None;

	if (MostRecentPropertyAddress)
		p_Z_Param_Out_SafeZoneInterfaceTemp = (TScriptInterface<IFortSafeZoneInterface>*)MostRecentPropertyAddress;

	if (Stack->Code())
		Stack->Step(Stack->Object(), &Z_Param_GamePhaseStep);
	else
	{
		FProperty* PropertyChainForCompiledIn = Stack->PropertyChainForCompiledIn();
		Stack->PropertyChainForCompiledIn() = (FProperty*)PropertyChainForCompiledIn->Next;
		Stack->StepExplicitProperty((unsigned __int8*)&Z_Param_GamePhaseStep, PropertyChainForCompiledIn);
	}

	unsigned __int8* Code = Stack->Code();
	Stack->Code() = &Code[Code != 0];

	FLIPPED_LOG("GamePhaseStep: ")
	FLIPPED_LOG((BYTE*)Z_Param_GamePhaseStep);

}

void execSpawnAI(UAthenaAIServicePlayerBots* Context, FFrame* Stack) {
	FLIPPED_LOG("Called");
}

TSubclassOf<AGameSession>* GetGameSessionClass(AFortGameModeAthena* thisPtr, TSubclassOf<AGameSession>* result)
{
	FLIPPED_LOG("GetGameSessionClass");
	result->ClassPtr = AFortGameSessionDedicatedAthena::StaticClass();
	return result;
}


const wchar_t* FCommandLine_GetCommandLine()
{
	static auto cmdLine = std::wstring(GetCommandLineOG()) + L" -AllowAllPlaylistsInShipping";
	return cmdLine.c_str();
}

const static FString ID = TEXT("ec684b8c687f479fadea3cb2ad83f5c6");

FServicePermissionsMcp* GetServicePermissionsByName(void* a1, void* a2)
{
	auto ServicePermissions = reinterpret_cast<TArray<FServicePermissionsMcp>*>(uint64(a1) + 0x98);
	if (!ServicePermissions || ServicePermissions->Num() == 0)
		return nullptr;

	FLIPPED_LOG(ServicePermissions->Num());

	FServicePermissionsMcp* Permissions = new FServicePermissionsMcp();
	Permissions->Id = ID;

	return Permissions;
}

void CreateAndConfigureNavSystem(UAthenaNavSystemConfig* Config, UWorld* World)
{
	FLIPPED_LOG("CreateAndConfigureNavSystem");
	Config->bPrioritizeNavigationAroundSpawners = true;
	Config->bAutoSpawnMissingNavData = true;
	Config->bLazyOctree = true;
	return CreateAndConfigureNavSystemOG(Config, World);
}

void InitalizeMMRInfos(UAthenaAIServicePlayerBots* thisPtr)
{
	FLIPPED_LOG("Called");
	auto AIService = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());
	AIService->DefaultBotAISpawnerData = Native::StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");

	FMMRSpawningInfo NewSpawningInfo{};
	NewSpawningInfo.BotSpawningDataInfoTargetELO = 1000.f; // todo: get proper value
	NewSpawningInfo.BotSpawningDataInfoWeight = 100.f; // todo: get proper value
	NewSpawningInfo.NumBotsToSpawn = 70;
	NewSpawningInfo.AISpawnerData = thisPtr->DefaultBotAISpawnerData;

	thisPtr->CachedMMRSpawningInfo.SpawningInfos.Add(NewSpawningInfo);
}

void WaitForMatchAssignmentReady(UAthenaAIServicePlayerBots* thisPtr, __int64 FlowHandle)
{
	auto GameState = UWorld::GetWorld()->GameState;

	FLIPPED_LOG(__FUNCTION__);

	*reinterpret_cast<int*>(__int64(thisPtr) + 0xB28) = GameState->PlayerArray.Num();

	return WaitForMatchAssignmentReadyOG(thisPtr, FlowHandle);
}

bool SpawnLoot(ABuildingContainer* Container) {

	if (bLategame)
		return false;

	auto GameState = Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
	auto GameMode = Util::Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	if (!GameState || !GameMode || !Container || Container->bAlreadySearched)
		return false;

	FName LootTierGroupToUse = Container->SearchLootTierGroup;

	for (auto& [SupportTierGroup, Redirect] : GameMode->RedirectAthenaLootTierGroups) {
		if (Container->SearchLootTierGroup == SupportTierGroup)
			LootTierGroupToUse = Redirect;
	}

	TArray<FFortItemEntry> Entries;
	Looting::PickLootDrops(UWorld::GetWorld(), &Entries, LootTierGroupToUse, GameState->WorldLevel);

	if (Entries.Num() <= 0)
		return false;

	FVector Location = Container->K2_GetActorLocation();
	Location.Z += 20;
	for (const FFortItemEntry& Entry : Entries) {
		FSpawnPickupData Data{};
		printf("Entry: %s\n", Entry.ItemDefinition->GetFullName().c_str());
		Data.ItemDefinition = Entry.ItemDefinition;
		Data.Count = Entry.Count;
		Data.Location = Location;
		Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Container;
		Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
		Looting::SpawnPickup(Data);
	}

	Container->bAlreadySearched = true;
	Container->OnRep_bAlreadySearched();
	Container->SearchBounceData.SearchAnimationCount++;
	Container->BounceContainer();

	return Container;
}

void ServerAttemptAircraftJump(UFortControllerComponent_Aircraft* thisPtr, const FRotator& Rotation) {
	AFortGameModeAthena* GameMode = Util::Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
	if (!GameMode)
		return;

	AFortPlayerControllerAthena* Controller = Util::Cast<AFortPlayerControllerAthena>(thisPtr->GetOwner());
	if (!Controller || !Controller->IsInAircraft())
		return;


	GameMode->RestartPlayer(Controller);
	Controller->ControlRotation = Rotation;
}


void OnAircraftEnteredDropZone(AFortGameModeAthena* thisPtr, AFortAthenaAircraft* Aircraft)
{
	if (!thisPtr || !Aircraft)
		return OnAircraftEnteredDropZoneOG(thisPtr, Aircraft);

	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(thisPtr->GameState);
	FVector_NetQuantize100 SafeZoneLocation = (FVector_NetQuantize100)thisPtr->SafeZoneLocations[4];

	SafeZoneLocation.Z += 10000;

	Aircraft->FlightInfo.FlightStartLocation = SafeZoneLocation;
	Aircraft->FlightStartTime = 1;
	Aircraft->FlightEndTime = 5;

	GameState->bAircraftIsLocked = false;
	Aircraft->FlightInfo.TimeTillDropStart = 2;
	Aircraft->K2_TeleportTo(SafeZoneLocation, Aircraft->K2_GetActorRotation());

	UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"skipaircraft", nullptr);
	GameState->SafeZonesStartTime = 1;

	OnAircraftEnteredDropZoneOG(thisPtr, Aircraft);
}


void ServerCreateBuildingActor(AFortPlayerControllerAthena* thisPtr, FCreateBuildingActorData& BuildingData) {
	if (!thisPtr || !thisPtr->Pawn || !thisPtr->PlayerState || thisPtr->IsInAircraft()) return;
	APawn* Pawn = thisPtr->Pawn;
	FVector Location = BuildingData.BuildLoc;

	if (Pawn->K2_GetActorLocation().GetDistanceToInMeters(Location) > 20)
	{
		Pawn->K2_DestroyActor();
		thisPtr->ClientReturnToMainMenu(L"Create a ticket in the flipped support server! Kick Code: 0x58893");
		return;
	}

	FRotator Rotation = BuildingData.BuildRot;
	UClass* BuildingClass = thisPtr->BroadcastRemoteClientInfo->RemoteBuildableClass.Get();
	if (!BuildingClass)
		return ServerCreateBuildingActorOG(thisPtr, BuildingData);

	ABuildingSMActor* BuildingClassObject = Util::Cast<ABuildingSMActor>(BuildingClass->DefaultObject);
	if (!BuildingClassObject)
		return ServerCreateBuildingActorOG(thisPtr, BuildingData);

	bool bMirrored = BuildingData.bMirrored;
	UFortResourceItemDefinition* ResourceDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(BuildingClassObject->ResourceType);
	if (!ResourceDefinition)
		return ServerCreateBuildingActorOG(thisPtr, BuildingData);

	TArray<ABuildingActor*> ExistingBuildings;
	char dih = 0;
	bool bCanBuild =
		Native::CanPlaceBuildableClassInStructuralGrid(GetWorld(), BuildingClass, Location, Rotation, bMirrored, &ExistingBuildings, &dih)
		== EFortStructuralGridQueryResults::CanAdd;

	if (!bCanBuild)
		return ServerCreateBuildingActorOG(thisPtr, BuildingData);

	for (ABuildingActor*& CurrentBuild : ExistingBuildings) CurrentBuild->K2_DestroyActor();
	if (ABuildingSMActor* NewBuilding = Misc::SpawnActor<ABuildingSMActor>(Location, Rotation, thisPtr, BuildingClass)) {
		Inventory::RemoveItem(thisPtr, ResourceDefinition, 10);

		NewBuilding->bPlayerPlaced = true;
		NewBuilding->SetMirrored(bMirrored);

		NewBuilding->TeamIndex = Util::Cast<AFortPlayerStateAthena>(thisPtr->PlayerState)->TeamIndex;
		NewBuilding->Team = EFortTeam(NewBuilding->TeamIndex);
		NewBuilding->OnRep_Team();

		NewBuilding->InitializeKismetSpawnedBuildingActor(NewBuilding, thisPtr, true, nullptr);
	}
	else 
	{
		ExistingBuildings.Free();
		return ServerCreateBuildingActorOG(thisPtr, BuildingData);
	}

	ExistingBuildings.Free();
}

void (*ServerRepairBuildingActorOG)(AFortPlayerControllerAthena*, ABuildingSMActor*);
void ServerRepairBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* ActorToRepair) {
	/* due to how c++ works even if ps is invalid it wont reach the team index check so ye, no crash */
	if (!Controller || !Controller->Pawn || !Controller->PlayerState || ActorToRepair->TeamIndex != ((AFortPlayerStateAthena*)Controller->PlayerState)->TeamIndex) return;

	int ToRemove = std::floor((10.f * (1.f - ActorToRepair->GetHealthPercent())) * 0.75f);
	UFortResourceItemDefinition* ResourceDefinition = UFortKismetLibrary::K2_GetResourceItemDefinition(ActorToRepair->ResourceType);
	if (!ResourceDefinition)
		return ServerRepairBuildingActorOG(Controller, ActorToRepair);

	bool bHasItemAndEnoughResources = false;
	for (size_t y = 0; y < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); y++) 
	{
		FFortItemEntry& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[y];
		if (Entry.ItemDefinition == ResourceDefinition) 
		{
			if (Entry.Count >= ToRemove) 
				bHasItemAndEnoughResources = true;
		}
	}

	if (bHasItemAndEnoughResources) 
	{
		Inventory::RemoveItem(Controller, ResourceDefinition, ToRemove);
		ActorToRepair->RepairBuilding(Controller, ToRemove);
	}
}

void (*ServerBeginEditingBuildingActorOG)(AFortPlayerControllerAthena*, ABuildingSMActor*);
void ServerBeginEditingBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* ActorToEdit) {
	if (!Controller || Controller->IsInAircraft() || !Controller->Pawn || ActorToEdit->TeamIndex != ((AFortPlayerStateAthena*)Controller->PlayerState)->TeamIndex) return;

	FGuid EditToolGuid = FGuid();
	for (size_t y = 0; y < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); y++) {
		auto& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[y];
		if (Entry.ItemDefinition->IsA(UFortEditToolItemDefinition::StaticClass())) {
			EditToolGuid = Entry.ItemGuid;
			break;
		}
	}

	/* need to add more shit heree but it works anyways so its fine!! */

	if (EditToolGuid == FGuid()) 
		return;

	Controller->ServerExecuteInventoryItem(EditToolGuid);
}

void (*ServerEditBuildingActorOG)(AFortPlayerControllerAthena*, ABuildingSMActor*);
void ServerEditBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* ActorToEdit, TSubclassOf<ABuildingSMActor> NewBuildingClass, uint8 RotationIterations, bool bMirrored) {
	if (!Controller || !ActorToEdit || !NewBuildingClass) return;

	ActorToEdit->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	ActorToEdit->EditingPlayer = nullptr;

	if (ABuildingSMActor* ReplaceActor = Native::ReplaceBuildingActor(ActorToEdit, 1, NewBuildingClass, ActorToEdit->CurrentBuildingLevel, RotationIterations, bMirrored, Controller))
		ReplaceActor->bPlayerPlaced = true;
}

void (*ServerEndEditingBuildingActorOG)(AFortPlayerControllerAthena*, ABuildingSMActor*);
void ServerEndEditingBuildingActor(AFortPlayerControllerAthena* Controller, ABuildingSMActor* ActorToStopEditing) {
	if (!Controller || !Controller->MyFortPawn || !ActorToStopEditing) return;

	ActorToStopEditing->SetNetDormancy(ENetDormancy::DORM_DormantAll);
	ActorToStopEditing->EditingPlayer = nullptr;
	ActorToStopEditing->OnRep_EditingPlayer();

	if (AFortWeap_EditingTool* EditTool = Util::Cast<AFortWeap_EditingTool>(Controller->MyFortPawn->CurrentWeapon)) {
		EditTool->bEditConfirmed = true;
		EditTool->EditActor = nullptr;
	}
}
