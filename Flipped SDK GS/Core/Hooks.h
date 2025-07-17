#pragma once
#include "../Other/Framework.h"
#include "./Inventory.h"
#include"./Looting.h"
#include "./AI.h"

void (*DispatchRequestOG)(__int64, __int64, int); void DispatchRequest(__int64 a1, __int64 a2, int a3) { return DispatchRequestOG(a1, a2, 3); }
void (*TickFlushOG)(UNetDriver*); void TickFlush(UNetDriver* Driver) { 
	if (Driver && Driver->ReplicationDriver && Driver->ClientConnections.Num() > 0) 
		Native::ServerReplicateActors(Driver->ReplicationDriver); 

	if (GetAsyncKeyState(VK_F5) & 1) {
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"startaircraft", nullptr);
	}

	if (GetAsyncKeyState(VK_F6) & 1) {
		UKismetSystemLibrary::ExecuteConsoleCommand(UWorld::GetWorld(), L"demospeed 5", nullptr);
	}

	if (GetAsyncKeyState(VK_F7) & 1 && Driver) {
		FTransform Transform = Driver->ClientConnections[0]->PlayerController->Pawn->GetTransform();
		AI::SpawnKlombo(Transform, 1);
	}

	if (GetAsyncKeyState(VK_F8) & 1) {
		static int Index = 0;
		auto TimeOfDayManager = AFortTimeOfDayManager::GetTimeOfDayManagerFromContext(UWorld::GetWorld());
		if (TimeOfDayManager) {
			TimeOfDayManager->DisableGlobalWeatherEvents = false;
			TimeOfDayManager->WeatherComponent->bWeatherDisabled = false;
			static void (*StartWeatherEvent)(UFortTimeOfDayWeatherComponent * Comp) = decltype(StartWeatherEvent)(Addresses::ImageBase + 0x6272D98);
			StartWeatherEvent(TimeOfDayManager->WeatherComponent);
		}
		else {
			printf("NO TODM");
		}
	}
	
	TickFlushOG(Driver); 
}
void (*StartNewSafeZonePhaseOG)(AFortGameModeAthena* GameMode, int32_t NewSafeZonePhase);
void (*OnAircraftEnteredDropZoneOG)(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft);
void (*OnAircraftExitedDropZoneOG)(AFortGameModeAthena* GameMode, AFortAthenaAircraft* Aircraft);
const wchar_t* (*GetCommandLineOG)();
void (*CreateAndConfigureNavSystemOG)(UAthenaNavSystemConfig*, UWorld*);
void (*WaitForMatchAssignmentReadyOG)(UAthenaAIServicePlayerBots*, __int64);
void (*ServerCreateBuildingActorOG)(AFortPlayerControllerAthena* thisPtr, FCreateBuildingActorData BuildingActorData);

static void LoadLateGameLoadouts()
{
#pragma region ARS
	static UFortWeaponRangedItemDefinition* GreenARDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_UC_Ore_T03.WID_Assault_Auto_Athena_UC_Ore_T03");
	int Count = 1;
	FLategameLoadout GreenAR{};
	GreenAR.Count = Count;
	GreenAR.Definition = GreenARDefinition;
	ARLoadouts.push_back(GreenAR);
	static UFortWeaponRangedItemDefinition* BlueARDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_Auto_Athena_R_Ore_T03.WID_Assault_Auto_Athena_R_Ore_T03");
	FLategameLoadout BlueAR{};
	BlueAR.Count = Count;
	BlueAR.Definition = BlueARDefinition;
	ARLoadouts.push_back(BlueAR);
	static UFortWeaponRangedItemDefinition* PurpleScarDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_VR_Ore_T03.WID_Assault_AutoHigh_Athena_VR_Ore_T03");
	FLategameLoadout PurpleScar{};
	PurpleScar.Count = Count;
	PurpleScar.Definition = PurpleScarDefinition;
	ARLoadouts.push_back(PurpleScar);
	static UFortWeaponRangedItemDefinition* GoldenScarDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Assault_AutoHigh_Athena_SR_Ore_T03.WID_Assault_AutoHigh_Athena_SR_Ore_T03");
	FLategameLoadout GoldenScar{};
	GoldenScar.Count = Count;
	GoldenScar.Definition = GoldenScarDefinition;
	ARLoadouts.push_back(GoldenScar);
#pragma endregion

#pragma region SHOTGUNS
	static UFortWeaponRangedItemDefinition* BluePumpDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_UC_Ore_T03.WID_Shotgun_Standard_Athena_UC_Ore_T03");
	FLategameLoadout BluePump{ BluePumpDefinition, Count };
	ShotgunLoadouts.push_back(BluePump);
	static UFortWeaponRangedItemDefinition* PurplePumpDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_VR_Ore_T03.WID_Shotgun_Standard_Athena_VR_Ore_T03");
	FLategameLoadout PurplePump{ PurplePumpDefinition, Count};
	ShotgunLoadouts.push_back(PurplePump);
	static UFortWeaponRangedItemDefinition* GoldenPumpDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Shotgun_Standard_Athena_SR_Ore_T03.WID_Shotgun_Standard_Athena_SR_Ore_T03");
	FLategameLoadout GoldenPump{ GoldenPumpDefinition, Count};
	ShotgunLoadouts.push_back(GoldenPump);
#pragma endregion

#pragma region THIRDSLOT
	static UFortWeaponRangedItemDefinition* BlueSMGDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_R_Ore_T03");
	FLategameLoadout BlueSMG{ BlueSMGDefinition, Count};
	SMGLoadouts.push_back(BlueSMG);
	static UFortWeaponRangedItemDefinition* PurpleSMGDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_VR_Ore_T03");
	FLategameLoadout PurpleSMG{ PurpleSMGDefinition, Count};
	SMGLoadouts.push_back(PurpleSMG);
	static UFortWeaponRangedItemDefinition* GoldenSMGDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03.WID_Pistol_AutoHeavyPDW_Athena_SR_Ore_T03");
	FLategameLoadout GoldenSMG{ GoldenSMGDefinition, Count };
	SMGLoadouts.push_back(GoldenSMG);
	static UFortWeaponRangedItemDefinition* FlintKnockDefinition = Native::StaticLoadObject<UFortWeaponRangedItemDefinition>("/Game/Athena/Items/Weapons/WID_Pistol_Flintlock_Athena_UC.WID_Pistol_Flintlock_Athena_UC");
	FLategameLoadout FlintLock{ FlintKnockDefinition, Count };
	SMGLoadouts.push_back(FlintLock);
#pragma endregion

#pragma region FIRSTCONSUMABLE
	static UFortItemDefinition* RiftToGoDefinition = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/RiftItem/Athena_Rift_Item.Athena_Rift_Item");
	Count = 2;
	FLategameLoadout RiftToGo{ RiftToGoDefinition, Count };
	FirstConsumableSlotLoadouts.push_back(RiftToGo);
	static UFortItemDefinition* ChugSplashDefinition = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/ChillBronco/Athena_ChillBronco.Athena_ChillBronco");
	Count = 6;
	FLategameLoadout ChugSplash{ ChugSplashDefinition, Count };
	FirstConsumableSlotLoadouts.push_back(ChugSplash);
	static UFortItemDefinition* SlurpJuiceDefinition = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Consumables/PurpleStuff/Athena_PurpleStuff.Athena_PurpleStuff");
	Count = 3;
	FLategameLoadout SlurpJuice{ SlurpJuiceDefinition, Count };
	FirstConsumableSlotLoadouts.push_back(SlurpJuice);
#pragma endregion

#pragma region SECONDCONSUMABLE
	SecondConsumableSlotLoadouts.push_back(RiftToGo);
	SecondConsumableSlotLoadouts.push_back(ChugSplash);
	SecondConsumableSlotLoadouts.push_back(SlurpJuice);
#pragma endregion
}

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
	GameMode->GameSession->MaxPlayers = 50;

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
			UObject::FindObject<UFortPlaylistAthena>(bCreative ? "FortPlaylistAthena Playlist_PlaygroundV2.Playlist_PlaygroundV2" : "FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo"))
		{
			SetPlaylist(thisPtr, Playlist);

			//Playlist->RespawnType = EAthenaRespawnType::InfiniteRespawn;

			thisPtr->AISettings = Playlist->AISettings;
			if (thisPtr->AISettings)
				thisPtr->AISettings->AIServices[1] = UAthenaAIServicePlayerBots::StaticClass();

			if (!bUsesGameSessions) {
				for (int i = 0; i < Playlist->AdditionalLevels.Num(); i++)
				{
					TSoftObjectPtr<UWorld> World = Playlist->AdditionalLevels[i];
					FString LevelName = UKismetStringLibrary::Conv_NameToString(World.ObjectID.AssetPathName);
					ULevelStreamingDynamic::LoadLevelInstance(GetWorld(), LevelName, {}, {}, nullptr, FString(), {});
					FAdditionalLevelStreamed NewLevel{ World.ObjectID.AssetPathName,false };
					GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
				}

				for (const TSoftObjectPtr < UWorld>& CurrentAdditionalLevelServerOnly : Playlist->AdditionalLevelsServerOnly)
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
			}

			thisPtr->PlaylistHotfixOriginalGCFrequency = Playlist->GarbageCollectionFrequency;

			thisPtr->bAllowSpectateAfterDeath = true;

			thisPtr->AIDirector = Misc::SpawnActor<AAthenaAIDirector>();
			thisPtr->AIDirector->Activate();

			if (!thisPtr->AIGoalManager)
				thisPtr->AIGoalManager = Misc::SpawnActor<AFortAIGoalManager>();

			if (!thisPtr->SpawningPolicyManager)
			{
				thisPtr->SpawningPolicyManager = Misc::SpawnActor<AFortAthenaSpawningPolicyManager>();
				thisPtr->SpawningPolicyManager->GameModeAthena = thisPtr;
				thisPtr->SpawningPolicyManager->GameStateAthena = GameState;
			}

			GameState->DefaultParachuteDeployTraceForGroundDistance = 10000;
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
			SET_TITLE("Flipped 19.10 - Listening!");
		}

	}

	if (bUsesGameSessions) {
		static bool bWaitedForPlaylist = false;
		if (!bWaitedForPlaylist) {
			if (!thisPtr->CurrentBucketId.IsValid())
				return false;

			FLIPPED_LOG(thisPtr->CurrentBucketId.ToString());
			FLIPPED_LOG(thisPtr->CurrentBucketId.Num());
			FLIPPED_LOG("JOBS");
			auto Dih = Misc::split(thisPtr->CurrentBucketId.ToString(), ":");
			auto playlistName = Dih[5];
			FLIPPED_LOG(playlistName);
			auto PathPart = Misc::split(playlistName, "_")[1];
			if (PathPart == "showdownalt") PathPart = "showdown";
			std::string Shit = "/Game/Athena/Playlists/" + (PathPart.starts_with("default") ? "" : (PathPart + "/")) + std::string(playlistName) + "." + std::string(playlistName);
			UFortPlaylistAthena* Playlist = Native::StaticLoadObject<UFortPlaylistAthena>(Shit);
			if (!Playlist)
			{
				FLIPPED_LOG("Playlist not found");
				return false;
			}
			SetPlaylist(thisPtr, Playlist);
			for (int i = 0; i < Playlist->AdditionalLevels.Num(); i++)
			{
				TSoftObjectPtr<UWorld> World = Playlist->AdditionalLevels[i];
				FString LevelName = UKismetStringLibrary::Conv_NameToString(World.ObjectID.AssetPathName);
				ULevelStreamingDynamic::LoadLevelInstance(GetWorld(), LevelName, {}, {}, nullptr, FString(), {});
				FAdditionalLevelStreamed NewLevel{ World.ObjectID.AssetPathName,false };
				GameState->AdditionalPlaylistLevelsStreamed.Add(NewLevel);
			}

			for (const TSoftObjectPtr < UWorld>& CurrentAdditionalLevelServerOnly : Playlist->AdditionalLevelsServerOnly)
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

				if (Actor->IsA(AFortPlayerStartWarmup::StaticClass()) || Actor->IsA(AFortPlayerStartCreative::StaticClass()))
				{
					bLoadedProperly = true;
					break;
				}
			}
		}

		if (!bLoadedProperly)
			return false;

		printf("Playlist: %s\n", GameState->CurrentPlaylistInfo.BasePlaylist->GetName().c_str());

		static UDataTable* AthenaRangedWeapons = Native::StaticLoadObject<UDataTable>("/Game/Athena/Items/Weapons/AthenaRangedWeapons.AthenaRangedWeapons");
		Misc::ApplyDataTablePatch(AthenaRangedWeapons);

		if (bLategame) {
			LoadLateGameLoadouts();
		}
		else {
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

			WarmupClass = Native::StaticLoadObject<UClass>("/Game/Athena/Environments/Blueprints/Tiered_Athena_FloorLoot_01.Tiered_Athena_FloorLoot_01_C");
			UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), WarmupClass, &WarmupActors);

			for (auto& WarmupActor : WarmupActors)
			{
				auto Container = (ABuildingContainer*)WarmupActor;

				Container->BP_SpawnLoot(nullptr);

				Container->K2_DestroyActor();
			}
			WarmupActors.Free();
		}



		static void* (*GameplayTags)() = decltype(GameplayTags)(Addresses::ImageBase + 0xC904C4);
		TMap<FGameplayTag, int>& TagMap = *reinterpret_cast<TMap<FGameplayTag, int>*>(__int64(GameplayTags()) + 0x2850);
		for (auto& [Tag, Val] : TagMap) {
			UE_LOG(LogFlipped, Log, "Tag: %s", Tag.ToString().c_str());
		}
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

	if (GameState->GamePhase > EAthenaGamePhase::Warmup) {
		Inventory::RemoveAllDroppableItems(NewPlayer);
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
					//FLIPPED_LOG("Apply SafeZoneDamage");
					for (auto& Key : Curve->Keys)
					{
						FSimpleCurveKey* KeyPtr = &Key;
						if (KeyPtr->Time == 0.f) KeyPtr->Value = 0.f;
					}
				}
			}
		}
		else FLIPPED_LOG("Invalid AthenaGameDataTable!");

		UCurveTable* LagerGameData = Native::StaticLoadObject<UCurveTable>("/Lager/DataTables/LagerGameData.LagerGameData");
		for (auto& [Key, Value] : LagerGameData->GetRowMap()) {
			if (Key.ToString() == "Default.Lager.Event.Weather.Tornado.Enabled" || Key.ToString() == "Default.Lager.Category.Tandem.Enabled") {
				//printf("Perhaps");
				auto Row = (FSimpleCurve*)Value;
				for (auto& Key : Row->Keys) {
					Key.Value = 1.0;
				}
			}
		}

		bFirst = true;
	}

	auto PlayerState = (AFortPlayerStateAthena*)NewPlayer->PlayerState;

	NewPlayer->GetQuestManager(ESubGame::Athena)->InitializeQuestAbilities(NewPawn);

	return NewPawn;
}


void StartNewSafeZonePhase(AFortGameModeAthena* GameMode, int32_t OverrideSafeZonePhase)
{
	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(GameMode->GameState);
	if (!GameState)
		return StartNewSafeZonePhaseOG(GameMode, OverrideSafeZonePhase);

	static int LategameSafeZonePhase = 2;

	static bool bDih = false;

	if (!bDih)
	{
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
		}

		bDih = true;
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

	GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime = GameState->GetServerWorldTimeSeconds() + ChosenWaitTime;
	
	if (GameMode->SafeZonePhase >= 0 && GameMode->SafeZonePhase < GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached.Num())
		ShrinkingTime = GameState->MapInfo->SafeZoneDefinition.ShrinkTimeCached[GameMode->SafeZonePhase];


	GameMode->SafeZoneIndicator->SafeZoneFinishShrinkTime = GameMode->SafeZoneIndicator->SafeZoneStartShrinkTime + ShrinkingTime;

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
	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
	if (thisPtr->CosmeticLoadoutPC.Character) {
		PlayerState->HeroType = thisPtr->CosmeticLoadoutPC.Character ? thisPtr->CosmeticLoadoutPC.Character->HeroDefinition : nullptr;
		UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);
	}
	Misc::ApplyModifiersToPlayer(thisPtr);
}

void ServerLoadingScreenDropped(AFortPlayerControllerAthena* thisPtr) 
{
	AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
	AFortPlayerStateAthena* PlayerState = Util::Cast<AFortPlayerStateAthena>(thisPtr->PlayerState);
	AFortPlayerPawnAthena* Pawn = Util::Cast<AFortPlayerPawnAthena>(thisPtr->MyFortPawn);
	if (!Pawn) return;
	void* InterfaceAddress = Native::GetInterfaceAddress(PlayerState, IAbilitySystemInterface::StaticClass());

	if (!InterfaceAddress)
		return;

	TScriptInterface<IAbilitySystemInterface> Script;
	Script.ObjectPointer = PlayerState;
	Script.InterfacePointer = InterfaceAddress;

	UFortAbilitySet* DefaultAbilitySet = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer");

	if (DefaultAbilitySet)
		UFortKismetLibrary::EquipFortAbilitySet(Script, DefaultAbilitySet, nullptr);
	else
		FLIPPED_LOG("Invalid DefaultAbilitySet!");

	thisPtr->StatManager = (UStatManager*)UGameplayStatics::SpawnObject(UStatManager::StaticClass(), thisPtr);

	if (bCreative) {
		AFortCreativePortalManager*& CreativeManager = GameState->CreativePortalManager;
		AFortAthenaCreativePortal* AvaliablePortal = nullptr;
		for (AFortAthenaCreativePortal* Portal : CreativeManager->AllPortals) {
			if (!Portal->LinkedVolume || Portal->LinkedVolume->VolumeState == ESpatialLoadingState::Ready)
				continue;
			AvaliablePortal = Portal;
			break;
		}

		if (!AvaliablePortal)
			return;

		AvaliablePortal->OwningPlayer = PlayerState->UniqueId;
		Native::UpdateStatus(AvaliablePortal);
		AvaliablePortal->OnRep_OwningPlayer();

		if (!AvaliablePortal->bPortalOpen) {
			AvaliablePortal->bPortalOpen = true;
			AvaliablePortal->OnRep_PortalOpen();
		}

		AvaliablePortal->PlayersReady.Add(PlayerState->UniqueId);

		AvaliablePortal->bUserInitiatedLoad = true;
		AvaliablePortal->bInErrorState = false;
		thisPtr->OwnedPortal = AvaliablePortal;


		AvaliablePortal->LinkedVolume->VolumeState = ESpatialLoadingState::Ready;

		auto Comp = Util::Cast<UPlaysetLevelStreamComponent>(
			AvaliablePortal->GetLinkedVolume()->GetComponentByClass(UPlaysetLevelStreamComponent::StaticClass()));

		auto PlaysetDih = Native::StaticLoadObject<UFortPlaysetItemDefinition>("/Game/Playsets/PID_Playset_60x60_Composed.PID_Playset_60x60_Composed");
		Comp->SetPlayset(PlaysetDih);
		Comp->bAutoLoadLevel = true;

		auto Comp2 = Util::Cast<UFortLevelSaveComponent>(
			AvaliablePortal->GetLinkedVolume()->GetComponentByClass(UFortLevelSaveComponent::StaticClass()));

		Comp2->AccountIdOfOwner = PlayerState->UniqueId;
		Comp2->bIsLoaded = true;
		Comp2->bAutoLoadFromRestrictedPlotDefinition = true;

		printf("Playset: %s", Comp->CurrentPlayset->GetFullName().c_str());
		Native::LoadPlayset(Comp);

		thisPtr->CreativePlotLinkedVolume = AvaliablePortal->LinkedVolume;
		thisPtr->OnRep_CreativePlotLinkedVolume();

		static UFortItemDefinition* PhoneItemDef = UObject::FindObject<UFortItemDefinition>("FortWeaponRangedItemDefinition WID_CreativeTool.WID_CreativeTool");
		printf("Item: %s\n", PhoneItemDef->GetFullName().c_str());
		Inventory::AddItem(thisPtr, PhoneItemDef);
	}

	FGameMemberInfo Info{};
	Info.MemberUniqueId = PlayerState->UniqueId;
	Info.SquadId = PlayerState->SquadId;
	Info.TeamIndex = PlayerState->TeamIndex;
	Info.ReplicationID = -1;
	Info.MostRecentArrayReplicationKey = -1;
	Info.ReplicationKey = -1;
	GameState->GameMemberInfoArray.Members.Add(Info);
	GameState->GameMemberInfoArray.MarkArrayDirty();

	auto Function = UObject::FindObject<UFunction>("Function GA_Creative_OnKillSiphon.GA_Creative_OnKillSiphon_C.GiveResourcesToPlayer");

	printf("ServerNotifications: %d", UFortRuntimeOptions::GetDefaultObj()->bUseServerTournamentPlacementNotifications);

	if (!thisPtr->MatchReport) {
		thisPtr->MatchReport = (UAthenaPlayerMatchReport*)UGameplayStatics::SpawnObject(UAthenaPlayerMatchReport::StaticClass(), thisPtr);
	}
}

void ServerExecuteInventoryItem(AFortPlayerControllerAthena* thisPtr, const FGuid& ItemGUID)
{
	if (!thisPtr || thisPtr->IsInAircraft())
		return;

	UFortWorldItem* ItemInstance = nullptr;

	for (auto& Instance : thisPtr->WorldInventory->Inventory.ItemInstances) {
		if (Instance->ItemEntry.ItemGuid == ItemGUID) {
			ItemInstance = Instance;
			break;
		}
	}

	if (!ItemInstance || !ItemInstance->ItemEntry.ItemDefinition)
		return;

	UFortItemDefinition* ItemDefinition = ItemInstance->ItemEntry.ItemDefinition;

	if (!ItemDefinition)
		return;


	UFortGadgetItemDefinition* GadgetItemDefinition = Util::Cast<UFortGadgetItemDefinition>(ItemDefinition);

	if (GadgetItemDefinition) {
		ItemDefinition = GadgetItemDefinition->GetWeaponItemDefinition();
	}

	UFortDecoItemDefinition* DecoItemDefinition = Util::Cast<UFortDecoItemDefinition>(ItemDefinition);

	if (DecoItemDefinition) {
		thisPtr->MyFortPawn->PickUpActor(nullptr, DecoItemDefinition);
		thisPtr->MyFortPawn->GetCurrentWeapon()->ItemEntryGuid = ItemGUID;

		if (thisPtr->MyFortPawn->GetCurrentWeapon()->IsA(AFortDecoTool_ContextTrap::StaticClass())) {
			Util::Cast<AFortDecoTool_ContextTrap>(thisPtr->MyFortPawn->GetCurrentWeapon())->ContextTrapItemDefinition = (UFortContextTrapItemDefinition*)DecoItemDefinition;
		}

		return;
	}

	UFortWeaponItemDefinition* WeaponItemDefinition = Util::Cast<UFortWeaponItemDefinition>(ItemInstance->ItemEntry.ItemDefinition);

	if (WeaponItemDefinition) {
		FGuid TrackerGuid{};
		thisPtr->MyFortPawn->EquipWeaponDefinition(WeaponItemDefinition, ItemInstance->ItemEntry.ItemGuid, TrackerGuid, false);
	}
}

void GetPlayerViewPointHook(AFortPlayerControllerAthena* PC, FVector& Location, FRotator& Rotation) {
	if (PC && PC->StateName == UKismetStringLibrary::Conv_StringToName(L"Spectating"))
	{
		Location = PC->LastSpectatorSyncLocation;
		Rotation = PC->LastSpectatorSyncRotation;
	}
	else if (PC) {
		Location = PC->GetViewTarget()->K2_GetActorLocation();
		Rotation = PC->GetControlRotation();
	}
}

DWORD WINAPI KillThread(LPVOID lpParam) {
	Sleep(4000);
	TerminateProcess(GetCurrentProcess(), 0);
	return 0;
}

void (*RemoveFromAlivePlayers)(AFortGameMode* GameMode, AFortPlayerController* DeadPC, AFortPlayerState* KillerState, AFortPawn* KillerPawn, UFortWeaponItemDefinition* KillerWeapon, EDeathCause, char) = decltype(RemoveFromAlivePlayers)(uint64_t(GetModuleHandle(0)) + 0x5F9D3A8);
inline void (*ClientOnPawnDiedOG)(AFortPlayerControllerAthena* PC, FFortPlayerDeathReport& DeathReport);
void ClientOnPawnDiedHook(AFortPlayerControllerAthena* PC, FFortPlayerDeathReport& DeathReport)
{
	auto GameState = Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
	auto DeadPawn = PC->MyFortPawn;
	auto DeadPlayerState = Util::Cast<AFortPlayerStateAthena>(PC->PlayerState);
	auto KillerPawn = DeathReport.KillerPawn;
	auto KillerPlayerState = Util::Cast<AFortPlayerStateAthena>(DeathReport.KillerPlayerState);
	auto GameMode = Util::Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
	auto KillerController = KillerPlayerState ? Util::Cast<AFortPlayerControllerAthena>(DeathReport.KillerPlayerState->GetPlayerController()) : nullptr;
	FVector DeathLocation = DeadPawn->K2_GetActorLocation();
	float Distance = DeadPawn->GetDistanceTo(KillerPawn);
	UFortWeaponItemDefinition* Item = DeathReport.KillerWeapon;

	if (!DeadPlayerState)
		return ClientOnPawnDiedOG(PC, DeathReport);

	std::string PlayerName = DeadPlayerState->GetPlayerName().ToString();

	DeadPlayerState->DeathInfo.bDBNO = DeadPawn->WasDBNOOnDeath();
	DeadPlayerState->DeathInfo.bInitialized = true;
	DeadPlayerState->DeathInfo.DeathTags = *reinterpret_cast<FGameplayTagContainer*>(__int64(DeadPawn) + 0x19e0);
	DeadPlayerState->DeathInfo.DeathCause = KillerPlayerState->ToDeathCause(DeadPlayerState->DeathInfo.DeathTags, DeadPlayerState->DeathInfo.bDBNO);
	DeadPlayerState->DeathInfo.DeathClassSlot = (uint8)DeadPlayerState->DeathInfo.DeathCause;
	DeadPlayerState->DeathInfo.DeathLocation = DeathLocation;
	DeadPlayerState->DeathInfo.Distance = Distance;
	DeadPlayerState->DeathInfo.FinisherOrDowner = KillerPlayerState ? DeathReport.KillerPlayerState : PC->PlayerState;
	DeadPlayerState->OnRep_DeathInfo();
	//add mutator stuff

	if (PC->WorldInventory) {
		for (size_t i = 0; i < PC->WorldInventory->Inventory.ItemInstances.Num(); i++) {
			if (PC->WorldInventory->Inventory.ItemInstances[i]->CanBeDropped()) {
				FSpawnPickupData SpawnPickupData{};
				SpawnPickupData.bRandomRotation = true;
				SpawnPickupData.ItemDefinition = PC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.ItemDefinition;
				SpawnPickupData.Count = PC->WorldInventory->Inventory.ItemInstances[i]->ItemEntry.Count;
				SpawnPickupData.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Player;
				SpawnPickupData.FortPickupSpawnSource = EFortPickupSpawnSource::PlayerElimination;
				SpawnPickupData.Location = DeathLocation;
				Inventory::SpawnPickup(SpawnPickupData);
				PC->WorldInventory->Inventory.ItemInstances.Remove(i);
				PC->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
				PC->WorldInventory->Inventory.MarkArrayDirty();
			}
		}

		Inventory::UpdateInventory(PC);
	}

	bool AllDied = true;
	for (auto Member : DeadPlayerState->PlayerTeam->TeamMembers) {
		if (Member != PC && ((AFortPlayerControllerAthena*)Member)->bMarkedAlive)
		{
			AllDied = false;
			break;
		}
	}

	if (AllDied) {
		for (AController* CurrentMember : DeadPlayerState->PlayerTeam->TeamMembers) {
			AFortPlayerControllerAthena* CurrentMemberPC = Util::Cast<AFortPlayerControllerAthena>(CurrentMember);
			if (!CurrentMemberPC) continue;

			AFortPlayerStateAthena* CurrentMemberPlayerState = Util::Cast<AFortPlayerStateAthena>(CurrentMemberPC->PlayerState);
			if (!CurrentMemberPlayerState) continue;

			CurrentMemberPlayerState->Place = GameState->PlayersLeft;
			CurrentMemberPlayerState->OnRep_Place();
			FAthenaRewardResult Result{};
			Result.TotalSeasonXpGained = CurrentMemberPC->XPComponent->TotalXpEarned;
			Result.TotalBookXpGained = CurrentMemberPC->XPComponent->TotalXpEarned;
			FAthenaMatchStats Stats{};
			FAthenaMatchTeamStats TeamStats{};
			TeamStats.Place = DeadPlayerState->Place;
			TeamStats.TotalPlayers = GameState->TotalPlayers;
			Stats.bIsValid = true;
			CurrentMemberPC->ClientSendEndBattleRoyaleMatchForPlayer(true, Result);
			CurrentMemberPC->ClientSendMatchStatsForPlayer(CurrentMemberPC->MatchReport->MatchStats);
			CurrentMemberPC->ClientSendTeamStatsForPlayer(TeamStats);
			std::wstring PlaylistId = std::to_wstring(Misc::GetCurrentPlaylist()->PlaylistId);
			TDelegate<void(const void*)> CallbackDelegate;
			//CurrentMemberPC->AthenaProfile->EndBattleRoyaleGameV2
		}
	}

	if (KillerPlayerState && DeadPlayerState && DeadPlayerState != KillerPlayerState) {
		PC->ClientReceiveKillNotification(KillerPlayerState, DeadPlayerState);
		KillerPlayerState->ClientReportKill(DeadPlayerState);
		KillerPlayerState->KillScore++;
		printf("Attempting to update current members");
		for (AController* CurrentMember : KillerPlayerState->PlayerTeam->TeamMembers) {
			AFortPlayerControllerAthena* CurrentMemberPC = Util::Cast<AFortPlayerControllerAthena>(CurrentMember);
			if (!CurrentMemberPC) continue;
			AFortPlayerStateAthena* CurrentMemberPlayerState = Util::Cast<AFortPlayerStateAthena>(CurrentMemberPC->PlayerState);
			if (!CurrentMemberPlayerState) continue;
			CurrentMemberPlayerState->TeamKillScore++;
			CurrentMemberPlayerState->OnRep_TeamKillScore();
			printf("ClientReportTeamKill");
			CurrentMemberPlayerState->ClientReportTeamKill(CurrentMemberPlayerState->TeamKillScore);
		}
		KillerPlayerState->OnRep_Kills();

		//KillerPlayerState->ClientReportTournamentStatUpdate
	}
	RemoveFromAlivePlayers(GameMode, PC, DeadPlayerState, KillerPawn, Item, DeadPlayerState->DeathInfo.DeathCause, 0);
	std::map<int, int> PlayersToPlacement;
	PlayersToPlacement.insert({ 1,50 });
	PlayersToPlacement.insert({ 2,25 });
	PlayersToPlacement.insert({ 3,10 });
	PlayersToPlacement.insert({ 4,10 });
	PlayersToPlacement.insert({ 5,20 });
	PlayersToPlacement.insert({ 10,10 });
	PlayersToPlacement.insert({ 20,15 });
	PlayersToPlacement.insert({ 25,15 });
	PlayersToPlacement.insert({ 30,10 });
	PlayersToPlacement.insert({ 40,10 });
	PlayersToPlacement.insert({ 50,10 }); /*should be 0 for people in champs*/

	if (!PlayersToPlacement.empty()) {
		for (auto& [PlayersLeft, PointsEarned] : PlayersToPlacement) {
			if (GameState->PlayersLeft == PlayersLeft) {
				for (const auto& AlivePlayer : GameMode->AlivePlayers) {
					printf("Player %s, Points Earned %d", AlivePlayer->GetFullName().c_str(), PointsEarned);
					AlivePlayer->ClientReportTournamentPlacementPointsScored(PlayersLeft, PointsEarned);
				}
			}
		}
	}
	else {
		printf("GG");
	}
	PC->bMarkedAlive = false;
	
	if (bUsesGameSessions) {
		std::string URL = "http://15.235.16.134:3551/results/endofmatch/" + PlayerName;
		int TotalKillsReward = DeadPlayerState->KillScore > 1 ? (DeadPlayerState->KillScore * 5) : 0;
		int Hype = 0;
		int Vbucks = TotalKillsReward /*kills*/ + (DeadPlayerState->Place == 1 ? 50 : 0);
		printf("Player %s has died, Hype Earned: %d, Vbucks Earned: %d.\n", PlayerName.c_str(), Hype, Vbucks);
		std::string JSON = "{"
			"\"xp\": " + std::to_string(1000) + ", "
			"\"vbucks\": " + std::to_string(Vbucks) + ", "
			"\"hype\": " + std::to_string(Hype) + "}";
		PostRequest(URL, JSON);
	}

	ClientOnPawnDiedOG(PC, DeathReport);


	if (GameMode->AlivePlayers.Num() <= 1)
	{
		CreateThread(0, 0, KillThread, 0, 0, 0);
	}

	if (KillerPawn && KillerPlayerState) {
		
		static auto Wood = Native::StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/WoodItemData.WoodItemData");
		static auto Stone = Native::StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/StoneItemData.StoneItemData");
		static auto Metal = Native::StaticLoadObject<UFortItemDefinition>("/Game/Items/ResourcePickups/MetalItemData.MetalItemData");

		Inventory::AddItem(PC, Wood, 50);
		Inventory::AddItem(PC, Stone, 50);
		Inventory::AddItem(PC, Metal, 50);

		static FGameplayTag EarnedElim = { UKismetStringLibrary::Conv_StringToName(TEXT("Event.EarnedElimination")) };
		FGameplayEventData Data{};
		Data.EventTag = EarnedElim;
		Data.ContextHandle = KillerPlayerState->AbilitySystemComponent->MakeEffectContext(); //need to be using MakeFortEffectContext
		Data.Instigator = KillerController;
		Data.Target = DeadPlayerState;
		Data.TargetData = UAbilitySystemBlueprintLibrary::AbilityTargetDataFromActor(DeadPlayerState);
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(KillerPawn, Data.EventTag, Data);
		printf("Test\n");
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
	EAthenaGamePhaseStep Z_Param_GamePhaseStep;

	Stack->StepCompiledIn(&z_Param_Out_SafeZoneInterfaceTemp);
	Stack->StepCompiledIn(&Z_Param_GamePhaseStep);

	unsigned __int8* Code = Stack->Code();
	if (Code) {
		Stack->Code() = &Code[Code != 0];
	}


	UE_LOG(LogFlipped, Log, "OnGamePhaseStepChanged called with SafeZoneInterface: %s, GamePhaseStep: %d", z_Param_Out_SafeZoneInterfaceTemp.GetObjectRef()->GetName().c_str(), (int)Z_Param_GamePhaseStep);

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
	Config->bAutoSpawnMissingNavData = true;
	return CreateAndConfigureNavSystemOG(Config, World);
}

void InitalizeMMRInfos(UAthenaAIServicePlayerBots* thisPtr)
{
	FLIPPED_LOG("Called");
	printf("AISERVICE: %s", thisPtr->GetFullName().c_str());

	UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());

	AIServicePlayerBots->DefaultBotAISpawnerData = Native::StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");

	printf("%s", AIServicePlayerBots->DefaultBotAISpawnerData->GetFullName().c_str());

	FMMRSpawningInfo NewSpawningInfo{};
	NewSpawningInfo.BotSpawningDataInfoTargetELO = 1000.f;
	NewSpawningInfo.BotSpawningDataInfoWeight = 100.f;
	NewSpawningInfo.NumBotsToSpawn = bDisableAI ? 0 : 15;
	NewSpawningInfo.AISpawnerData = AIServicePlayerBots->DefaultBotAISpawnerData;

	AIServicePlayerBots->DefaultAISpawnerDataComponentList = UFortAthenaAISpawnerData::CreateComponentListFromClass(AIServicePlayerBots->DefaultBotAISpawnerData, UWorld::GetWorld());
	AIServicePlayerBots->CachedMMRSpawningInfo.SpawningInfos.Add(NewSpawningInfo);
	AIServicePlayerBots->GamePhaseToStartSpawning = EAthenaGamePhase::Warmup;

	printf("CachedFoundations: %d\n", AIServicePlayerBots->CachedBuildingFoundations.Num());

	printf("POI: %d\n", AIServicePlayerBots->CachedValidPOIVolumeLocations.Num());

	printf("Dih: %d\n", AIServicePlayerBots->GamePhaseToStartSpawning);

}

void WaitForMatchAssignmentReady(UAthenaAIServicePlayerBots* thisPtr, __int64 FlowHandle)
{
	auto GameState = UWorld::GetWorld()->GameState;

	FLIPPED_LOG(__FUNCTION__);

	*reinterpret_cast<int*>(__int64(thisPtr) + 0xB28) = GameState->PlayerArray.Num();

	return WaitForMatchAssignmentReadyOG(thisPtr, FlowHandle);
}

struct FReservedRandomValues
{
	TArray<float> RandomValues;
	int32 CurrentIndex;
	int32 NumReservedRandomValues;
};

__int64 (*PostUpdateOG)(ABuildingContainer* Container, uint32_t a2, FReservedRandomValues* a3);
__int64 PostUpdate(ABuildingContainer* Container, uint32_t a2, FReservedRandomValues* a3)
{
	Container->ReplicatedLootTier = 1;
	Container->OnRep_LootTier();

	return PostUpdateOG(Container, a2, a3);
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
		if (LootTierGroupToUse == SupportTierGroup) {
			LootTierGroupToUse = Redirect;
			break;
		}
	}

	//printf("LootTierGroupToUse: %s", LootTierGroupToUse.ToString().c_str());

	FVector Location = Container->K2_GetActorLocation() + 
		(Container->GetActorForwardVector() * Container->LootSpawnLocation_Athena.X) + 
		(Container->GetActorRightVector() * Container->LootSpawnLocation_Athena.Y) + 
		(Container->GetActorUpVector() * Container->LootSpawnLocation_Athena.Z);

	std::string LootTierGroupStr = LootTierGroupToUse.ToString();

	for (const FFortItemEntry& Entry : Looting::PickLootDrops(LootTierGroupStr, !LootTierGroupToUse.ToString().contains("Loot_AthenaFloorLoot"))) {
		FSpawnPickupData Data{};
		Data.ItemDefinition = Entry.ItemDefinition;
		Data.Count = Entry.Count;
		Data.Location = Location;
		Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Container;
		Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
		Inventory::SpawnPickup(Data);
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

	printf("ServerAttemptAircraftJump");

	AFortPlayerControllerAthena* Controller = Util::Cast<AFortPlayerControllerAthena>(thisPtr->GetOwner());
	if (!Controller || !Controller->IsInAircraft())
		return;



	GameMode->RestartPlayer(Controller);
	Controller->ControlRotation = Rotation;

	if (bLategame) {
		int32 ArLoadoutIndex = UKismetMathLibrary::RandomIntegerInRange(0, ARLoadouts.size() - 1);
		FLategameLoadout* ARLoadout = &ARLoadouts.at(ArLoadoutIndex);
		Inventory::AddItem(Controller, ARLoadout->Definition, ARLoadout->Count);
		int32 ShotgunLoadoutIndex = UKismetMathLibrary::RandomIntegerInRange(0, ShotgunLoadouts.size() - 1);
		FLategameLoadout* ShotgunLoadout = &ShotgunLoadouts.at(ShotgunLoadoutIndex);
		Inventory::AddItem(Controller, ShotgunLoadout->Definition, ShotgunLoadout->Count);
		int32 SMGLoadoutIndex = UKismetMathLibrary::RandomIntegerInRange(0, SMGLoadouts.size() - 1);
		FLategameLoadout* thirdSlot = &SMGLoadouts.at(SMGLoadoutIndex);
		Inventory::AddItem(Controller, thirdSlot->Definition, thirdSlot->Count);
		int32 FirstConsumableIndex = UKismetMathLibrary::RandomIntegerInRange(0, FirstConsumableSlotLoadouts.size() - 1);
		FLategameLoadout* FirstConsumable = &FirstConsumableSlotLoadouts.at(FirstConsumableIndex);
		Inventory::AddItem(Controller, FirstConsumable->Definition, FirstConsumable->Count);
		int32 SecondConsumableIndex = UKismetMathLibrary::RandomIntegerInRange(0, SecondConsumableSlotLoadouts.size() - 1);
		FLategameLoadout* SecondConsumable = &SecondConsumableSlotLoadouts.at(SecondConsumableIndex);
		Inventory::AddItem(Controller, SecondConsumable->Definition, SecondConsumable->Count);

		//AMMO
		static UFortItemDefinition* HeavyBullets = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsHeavy.AthenaAmmoDataBulletsHeavy");
		Inventory::AddItem(Controller, HeavyBullets, 999);
		static UFortItemDefinition* LightBullets = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsLight.AthenaAmmoDataBulletsLight");
		Inventory::AddItem(Controller, LightBullets, 999);
		static UFortItemDefinition* MediumBullets = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataBulletsMedium.AthenaAmmoDataBulletsMedium");
		Inventory::AddItem(Controller, MediumBullets, 999);
		static UFortItemDefinition* ShotgunBullets = Native::StaticLoadObject<UFortItemDefinition>("/Game/Athena/Items/Ammo/AthenaAmmoDataShells.AthenaAmmoDataShells");
		Inventory::AddItem(Controller, ShotgunBullets, 999);
		static UFortItemDefinition* Wood = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Wood);
		static UFortItemDefinition* Stone = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Stone);
		static UFortItemDefinition* Metal = UFortKismetLibrary::K2_GetResourceItemDefinition(EFortResourceType::Metal);
		Inventory::AddItem(Controller, Wood, 500);
		Inventory::AddItem(Controller, Stone, 450);
		Inventory::AddItem(Controller, Metal, 350);

		Controller->MyFortPawn->SetShield(100);
	}
}


void OnAircraftEnteredDropZone(AFortGameModeAthena* thisPtr, AFortAthenaAircraft* Aircraft)
{
	if (!thisPtr || !Aircraft || !bLategame)
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

	//thisPtr->OnAircraftExitedDropZone(Aircraft);
	GameState->SafeZonesStartTime = 1;

	OnAircraftEnteredDropZoneOG(thisPtr, Aircraft);
}

void OnAircraftExitedDropZone(AFortGameModeAthena* thisPtr, AFortAthenaAircraft* Aircraft)
{
	UAthenaAIServicePlayerBots* AIServicePlayerBots = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());
	if (!bLategame)
		return OnAircraftExitedDropZoneOG(thisPtr, Aircraft);
	auto GameState = Util::Cast<AFortGameStateAthena>(thisPtr->GameState);

	auto PlayerControllers = UFortKismetLibrary::GetAllFortPlayerControllers(UWorld::GetWorld(), true, false);
	for (auto& PC : PlayerControllers) {
		PC->GetAircraftComponent()->ServerAttemptAircraftJump({});
	}

	OnAircraftExitedDropZoneOG(thisPtr, Aircraft);
	GameState->SafeZonesStartTime = 1;


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

void (*ProcessEventOG)(UObject*, UFunction*, void*);
void ProcessEvent(UObject* Context, UFunction* Function, void* Params) {

	if (Context && Function) {
		auto FunctionName = Function->GetName();
		auto FunctionFullName = Function->GetFullName();
		auto ObjectName = Context->GetName();

		if (!strstr(FunctionName.c_str(), ("EvaluateGraphExposedInputs")) &&
			!strstr(FunctionName.c_str(), ("Tick")) &&
			!strstr(FunctionName.c_str(), ("OnSubmixEnvelope")) &&
			!strstr(FunctionName.c_str(), ("OnSubmixSpectralAnalysis")) &&
			!strstr(FunctionName.c_str(), ("OnMouse")) &&
			!strstr(FunctionName.c_str(), ("Pulse")) &&
			!strstr(FunctionName.c_str(), ("BlueprintUpdateAnimation")) &&
			!strstr(FunctionName.c_str(), ("BlueprintPostEvaluateAnimation")) &&
			!strstr(FunctionName.c_str(), ("BlueprintModifyCamera")) &&
			!strstr(FunctionName.c_str(), ("BlueprintModifyPostProcess")) &&
			!strstr(FunctionName.c_str(), ("Loop Animation Curve")) &&
			!strstr(FunctionName.c_str(), ("UpdateTime")) &&
			!strstr(FunctionName.c_str(), ("GetMutatorByClass")) &&
			!strstr(FunctionName.c_str(), ("UpdatePreviousPositionAndVelocity")) &&
			!strstr(FunctionName.c_str(), ("IsCachedIsProjectileWeapon")) &&
			!strstr(FunctionName.c_str(), ("LockOn")) &&
			!strstr(FunctionName.c_str(), ("GetAbilityTargetingLevel")) &&
			!strstr(FunctionName.c_str(), ("ReadyToEndMatch")) &&
			!strstr(FunctionName.c_str(), ("ReceiveDrawHUD")) &&
			!strstr(FunctionName.c_str(), ("OnUpdateDirectionalLightForTimeOfDay")) &&
			!strstr(FunctionName.c_str(), ("GetSubtitleVisibility")) &&
			!strstr(FunctionName.c_str(), ("GetValue")) &&
			!strstr(FunctionName.c_str(), ("InputAxisKeyEvent")) &&
			!strstr(FunctionName.c_str(), ("ServerTouchActiveTime")) &&
			!strstr(FunctionName.c_str(), ("SM_IceCube_Blueprint_C")) &&
			!strstr(FunctionName.c_str(), ("OnHovered")) &&
			!strstr(FunctionName.c_str(), ("OnCurrentTextStyleChanged")) &&
			!strstr(FunctionName.c_str(), ("OnButtonHovered")) &&
			!strstr(FunctionName.c_str(), ("ExecuteUbergraph_ThreatPostProcessManagerAndParticleBlueprint")) &&
			!strstr(FunctionName.c_str(), "PinkOatmeal") &&
			!strstr(FunctionName.c_str(), "CheckForDancingAtFish") &&
			!strstr(FunctionName.c_str(), ("UpdateCamera")) &&
			!strstr(FunctionName.c_str(), ("GetMutatorContext")) &&
			!strstr(FunctionName.c_str(), ("CanJumpInternal")) &&
			!strstr(FunctionName.c_str(), ("OnDayPhaseChanged")) &&
			!strstr(FunctionName.c_str(), ("Chime")) &&
				!strstr(FunctionName.c_str(), ("ServerMove")) &&
				!strstr(FunctionName.c_str(), ("OnVisibilitySetEvent")) &&
				!strstr(FunctionName.c_str(), "ReceiveHit") &&
				!strstr(FunctionName.c_str(), "ReadyToStartMatch") &&
				!strstr(FunctionName.c_str(), "K2_GetComponentToWorld") &&
				!strstr(FunctionName.c_str(), "ClientAckGoodMove") &&
				!strstr(FunctionName.c_str(), "Prop_WildWest_WoodenWindmill_01") &&
				!strstr(FunctionName.c_str(), "ContrailCheck") &&
				!strstr(FunctionName.c_str(), "B_StockBattleBus_C") &&
				!strstr(FunctionName.c_str(), "Subtitles.Subtitles_C.") &&
				!strstr(FunctionName.c_str(), "/PinkOatmeal/PinkOatmeal_") &&
				!strstr(FunctionName.c_str(), "BP_SpectatorPawn_C") &&
				!strstr(FunctionName.c_str(), "FastSharedReplication") &&
				!strstr(FunctionName.c_str(), "OnCollisionHitEffects") &&
				!strstr(FunctionName.c_str(), "BndEvt__SkeletalMesh") &&
				!strstr(FunctionName.c_str(), ".FortAnimInstance.AnimNotify_") &&
				!strstr(FunctionName.c_str(), "OnBounceAnimationUpdate") &&
				!strstr(FunctionName.c_str(), "ShouldShowSoundIndicator") &&
				!strstr(FunctionName.c_str(), "Primitive_Structure_AmbAudioComponent_C") &&
				!strstr(FunctionName.c_str(), "PlayStoppedIdleRotationAudio") &&
				!strstr(FunctionName.c_str(), "UpdateOverheatCosmetics") &&
				!strstr(FunctionName.c_str(), "StormFadeTimeline__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "BindVolumeEvents") &&
				!strstr(FunctionName.c_str(), "UpdateStateEvent") &&
				!strstr(FunctionName.c_str(), "VISUALS__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "Flash__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "SetCollisionEnabled") &&
				!strstr(FunctionName.c_str(), "SetIntensity") &&
				!strstr(FunctionName.c_str(), "Storm__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "CloudsTimeline__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "SetRenderCustomDepth") &&
				!strstr(FunctionName.c_str(), "K2_UpdateCustomMovement") &&
				!strstr(FunctionName.c_str(), "AthenaHitPointBar_C.Update") &&
				!strstr(FunctionName.c_str(), "ExecuteUbergraph_Farm_WeatherVane_01") &&
				!strstr(FunctionName.c_str(), "HandleOnHUDElementVisibilityChanged") &&
				!strstr(FunctionName.c_str(), "ExecuteUbergraph_Fog_Machine") &&
				!strstr(FunctionName.c_str(), "ReceiveBeginPlay") &&
				!strstr(FunctionName.c_str(), "OnMatchStarted") &&
				!strstr(FunctionName.c_str(), "CustomStateChanged") &&
				!strstr(FunctionName.c_str(), "OnBuildingActorInitialized") &&
				!strstr(FunctionName.c_str(), "OnWorldReady") &&
				!strstr(FunctionName.c_str(), "OnAttachToBuilding") &&
				!strstr(FunctionName.c_str(), "Clown Spinner") &&
				!strstr(FunctionName.c_str(), "K2_GetActorLocation") &&
				!strstr(FunctionName.c_str(), "GetViewTarget") &&
				!strstr(FunctionName.c_str(), "GetAllActorsOfClass") &&
				!strstr(FunctionName.c_str(), "OnUpdateMusic") &&
				!strstr(FunctionName.c_str(), "Check Closest Point") &&
				!strstr(FunctionName.c_str(), "OnSubtitleChanged__DelegateSignature") &&
				!strstr(FunctionName.c_str(), "OnServerBounceCallback") &&
				!strstr(FunctionName.c_str(), "BlueprintGetInteractionTime") &&
				!strstr(FunctionName.c_str(), "OnServerStopCallback") &&
				!strstr(FunctionName.c_str(), "Light Flash Timeline__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "MainFlightPath__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "PlayStartedIdleRotationAudio") &&
				!strstr(FunctionName.c_str(), "BGA_Athena_FlopperSpawn_") &&
				!strstr(FunctionName.c_str(), "CheckShouldDisplayUI") &&
				!strstr(FunctionName.c_str(), "Timeline_0__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "ClientMoveResponsePacked") &&
				!strstr(FunctionName.c_str(), "ExecuteUbergraph_B_Athena_FlopperSpawnWorld_Placement") &&
				!strstr(FunctionName.c_str(), "Countdown__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "OnParachuteTrailUpdated") &&
				!strstr(FunctionName.c_str(), "Moto FadeOut__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "ExecuteUbergraph_Apollo_GasPump_Valet") &&
				!strstr(FunctionName.c_str(), "GetOverrideMeshMaterial") &&
				!strstr(FunctionName.c_str(), "VendWobble__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "WaitForPawn") &&
				!strstr(FunctionName.c_str(), "FragmentMovement__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "TrySetup") &&
				!strstr(FunctionName.c_str(), "Fade Doused Smoke__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "SetPlayerToSkydive") &&
				!strstr(FunctionName.c_str(), "BounceCar__UpdateFunc") &&
				!strstr(FunctionName.c_str(), "BP_CalendarDynamicPOISelect") &&
				!strstr(FunctionName.c_str(), "OnComponentHit_Event_0") &&
				!strstr(FunctionName.c_str(), "HandleSimulatingComponentHit") &&
				!strstr(FunctionName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
				!strstr(FunctionName.c_str(), "WarmupCountdownEndTimeUpdated") &&
				!strstr(FunctionName.c_str(), "BP_CanInteract") &&
				!strstr(FunctionName.c_str(), "AthenaHitPointBar_C") &&
				!strstr(FunctionName.c_str(), "ServerFireAIDirectorEvent") &&
				!strstr(FunctionName.c_str(), "BlueprintThreadSafeUpdateAnimation") &&
				!strstr(FunctionName.c_str(), "On Amb Zap Spawn") &&
				!strstr(FunctionName.c_str(), "ServerSetPlayerCanDBNORevive") &&
				!strstr(FunctionName.c_str(), "BGA_Petrol_Pickup_C") &&
				!strstr(FunctionName.c_str(), "GetMutatorsForContextActor") &&
				!strstr(FunctionName.c_str(), "GetControlRotation") &&
				!strstr(FunctionName.c_str(), "K2_GetComponentLocation") &&
				!strstr(FunctionName.c_str(), "MoveFromOffset__UpdateFunc") &&
				!strstr(FunctionFullName.c_str(), "PinkOatmeal_GreenGlop_C") &&
				!strstr(ObjectName.c_str(), "CBGA_GreenGlop_WithGrav_C") &&
				!strstr(ObjectName.c_str(), "FlopperSpawn") &&
				!strstr(FunctionFullName.c_str(), "GCNL_EnvCampFire_Fire_C") &&
				!strstr(FunctionName.c_str(), "BlueprintGetAllHighlightableComponents") &&
				!strstr(FunctionFullName.c_str(), "Primitive_Structure_AmbAudioComponent") &&
				!strstr(FunctionName.c_str(), "ServerTriggerCombatEvent") &&
				!strstr(FunctionName.c_str(), "SpinCubeTimeline__UpdateFunc") &&
				!strstr(ObjectName.c_str(), "FortPhysicsObjectComponent") &&
				!strstr(FunctionName.c_str(), "GetTextValue") &&
				!strstr(FunctionName.c_str(), "ExecuteUbergraph_BGA_Petrol_Pickup") &&
				!strstr(FunctionName.c_str(), "Execute") && Context->IsA(ANPC_Pawn_Wildlife_Parent_C::StaticClass()))

		{
			printf(__FUNCTION__" Function called: %s with %s\n", FunctionFullName.c_str(), ObjectName.c_str());
		}
	}

	return ProcessEventOG(Context, Function, Params);
}

//Creative Stuff
void TeleportPlayer(AFortAthenaCreativePortal* Context, FFrame* Stack)
{
	printf(__FUNCTION__);
	if (!Context)
		return;
	AFortPlayerPawn* PlayerPawn = nullptr;
	FRotator TeleportRotation;
	Stack->Step(Stack->Object(), &PlayerPawn);
	Stack->Step(Stack->Object(), &TeleportRotation);

	if (!PlayerPawn)
		return;

	unsigned __int8* Code = Stack->Code();
	Stack->Code() = &Code[Code != 0];

	PlayerPawn->K2_TeleportTo(Context->TeleportLocation, TeleportRotation);
}

void TeleportPlayerToLinkedVolume(AFortAthenaCreativePortal* Context, FFrame* Stack, void* Ret)
{
	printf(__FUNCTION__"\n");

	AFortPlayerPawn* PlayerPawn = nullptr;
	bool bUseSpawnTags;

	Stack->Step(Stack->Object(), & PlayerPawn);
	Stack->Step(Stack->Object(), & bUseSpawnTags);

	if (!PlayerPawn)
		return;

	unsigned __int8* Code = Stack->Code();
	Stack->Code() = &Code[Code != 0];


	FVector Location = Context->LinkedVolume->K2_GetActorLocation();
	Location.Z += 10000;
	FTransform Transform{};
	Transform.Translation = Location;
	PlayerPawn->K2_TeleportTo(Location, {});
	//UWorld::GetWorld()->AuthorityGameMode->RestartPlayerAtTransform(PlayerPawn->Controller, Transform);

}

void ServerGiveCreativeItem(AFortPlayerControllerAthena* Controller, FFortItemEntry& ItemEntry) {
	Inventory::AddItem(Controller, ItemEntry.ItemDefinition, ItemEntry.Count);
}

void (*ServerPlayEmoteItemOG)(AFortPlayerControllerAthena* PC, UFortMontageItemDefinitionBase* EmoteAsset);
void ServerPlayEmoteItem(AFortPlayerControllerAthena* PC, UFortMontageItemDefinitionBase* EmoteAsset) {
	if (!PC || !EmoteAsset) return;

	UAbilitySystemComponent* ASC = Util::Cast<AFortPlayerStateAthena>(PC->PlayerState)->AbilitySystemComponent;
	FGameplayAbilitySpec* SpecPtr = nullptr;
	FGameplayAbilitySpec Spec{ -1,-1,-1 };
	if (auto DanceAsset = Util::Cast<UAthenaDanceItemDefinition>(EmoteAsset)) {
		PC->MyFortPawn->bMovingEmote = DanceAsset->bMovingEmote;
		PC->MyFortPawn->bMovingEmoteForwardOnly = DanceAsset->bMoveForwardOnly;
		PC->MyFortPawn->EmoteWalkSpeed = DanceAsset->WalkForwardSpeed;
		Spec.Ability = (UGameplayAbility*)UGAB_Emote_Generic_C::StaticClass()->DefaultObject;
		Spec.Level = 1;
		Spec.InputID = -1;
		Spec.SourceObject = (UObject*)EmoteAsset;
		Spec.Handle.Handle = rand();
		SpecPtr = &Spec;

	}
	if (EmoteAsset->IsA(UAthenaSprayItemDefinition::StaticClass())) {
		Spec.Ability = (UGameplayAbility*)UGAB_Spray_Generic_C::StaticClass()->DefaultObject;
		Spec.Level = 1;
		Spec.InputID = -1;
		Spec.SourceObject = (UObject*)EmoteAsset;
		Spec.Handle.Handle = rand();
		SpecPtr = &Spec;
	}
	if (EmoteAsset->IsA(UAthenaToyItemDefinition::StaticClass())) {
		UClass* ToyThingy = Util::Cast<UAthenaToyItemDefinition>(EmoteAsset)->ToySpawnAbility.NewGet();
		Spec.Ability = (UGameplayAbility*)ToyThingy->DefaultObject;
		Spec.Level = 1;
		Spec.InputID = -1;
		Spec.SourceObject = (UObject*)EmoteAsset;
		Spec.Handle.Handle = rand();
		SpecPtr = &Spec;
	}

	int OutHandle = 0;
	Native::GiveAbilityAndActivateOnce(ASC, &OutHandle, SpecPtr, nullptr);

	return ServerPlayEmoteItemOG(PC, EmoteAsset);
}

void (*MovingEmoteStoppedOG)(AFortPawn*, FFrame*, void*);
void MovingEmoteStopped(AFortPawn* Pawn, FFrame* Stack, void* Ret) {
	Pawn->bMovingEmote = false;
	Pawn->bMovingEmoteForwardOnly = false;

	return MovingEmoteStoppedOG(Pawn, Stack, Ret);
}

void OnCapsuleBeginOverlapHook(AFortPlayerPawnAthena* Context, FFrame* Stack, void* Ret)
{
	UPrimitiveComponent* OverlappedComp = nullptr;
	AActor* OtherActor = nullptr;
	UPrimitiveComponent* OtherComp = nullptr;
	int OtherBodyIndex;
	bool bFromSweep;
	FHitResult SweepResult;

	if (!Context)
	{
		printf("No Context??");
		return;
	}
	Stack->StepCompiledIn(&OverlappedComp);
	Stack->StepCompiledIn(&OtherActor);
	Stack->StepCompiledIn(&OtherComp);
	Stack->StepCompiledIn(&OtherBodyIndex);
	Stack->StepCompiledIn(&bFromSweep);
	Stack->StepCompiledIn(&SweepResult);


	typedef ABGA_RiftPortal_Item_Athena_C Paster;

	if (OtherActor && OtherActor->IsA(AFortPickupAthena::StaticClass())) {
		AFortPickupAthena* Pickup = Util::Cast<AFortPickupAthena>(OtherActor);
		if (Pickup->PawnWhoDroppedPickup != Context)
		{
			if (!Pickup->PrimaryPickupItemEntry.ItemDefinition)
				return;
			if (Inventory::GetQuickbar(Pickup->PrimaryPickupItemEntry.ItemDefinition) != EFortQuickBars::Primary) {
				FFortPickupRequestInfo Info{};
				Info.bPlayPickupSound = false;
				Info.bIsAutoPickup = true;
				Info.bIsVisualOnlyPickup = false;
				Info.FlyTime = 0.4;
				Context->ServerHandlePickupInfo(Pickup, Info);
			}
		}
	}
	else if (OtherActor && OtherActor->IsA(Paster::StaticClass())) {
		auto RiftPortal = (Paster*)OtherActor;
		FVector TeleportLoc = RiftPortal->TeleportLocation;
		//printf("TeleportLoc: X:%f, Y:%f, Z:%f\n", TeleportLoc.X, TeleportLoc.Y, TeleportLoc.Z);
		Context->K2_TeleportTo(TeleportLoc, RiftPortal->ActorRotation);
		Context->BeginSkydiving(true);
		auto Func = RiftPortal->Class->GetFunction("BGA_RiftPortal_Item_Athena_C", "TeleportPlayerAndSendEvent");
		//printf("ExecPtr: %p\n", Func->ExecFunction);
	}
	else if (OtherActor) {
		//printf("OtherActor: %s\n", OtherActor->GetFullName().c_str());
	}
}

void ServerAttemptInventoryDrop(AFortPlayerControllerAthena* PlayerController, FGuid ItemGuid, int32 Count)
{
	//printf(__FUNCTION__"\n");
	auto ItemInstance = Inventory::GetItemFromGUID(PlayerController, ItemGuid);
	FSpawnPickupData Data{ };
	Data.bRandomRotation = true;
	Data.ItemDefinition = ItemInstance->ItemEntry.ItemDefinition;
	Data.Count = Count;
	Data.Location = PlayerController->MyFortPawn->K2_GetActorLocation();
	Data.PickupOwner = PlayerController->MyFortPawn;
	Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
	Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Player;
	Inventory::SpawnPickup(Data);
	Inventory::RemoveItem(PlayerController, ItemGuid, Count);
}

// TScriptInterface<class IFortInventoryOwnerInterface>InventoryOwner                                         (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, UObjectWrapper, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// const class UFortWorldItemDefinition*   ItemDefinition                                         (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// const struct FGuid&                     ItemVariantGuid                                        (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// int32                                   NumberToGive                                           (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// bool                                    bNotifyPlayer                                          (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// int32                                   ItemLevel                                              (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// int32                                   PickupInstigatorHandle                                 (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// bool                                    bUseItemPickupAnalyticEvent                            (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// const class UFortWorldItem*             ReturnValue                                            (ConstParm, Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
UFortWorldItem* execGiveItemToInventoryOwner(UFortKismetLibrary* Context, FFrame* Stack, UFortWorldItem** Ret) {
	printf(__FUNCTION__"\n");
	TScriptInterface<IFortInventoryOwnerInterface> InventoryOwner;
	UFortWorldItemDefinition* ItemDefinition = nullptr;
	FGuid ItemVariantGuid;
	int32 NumberToGive;
	bool bNotifyPlayer;
	int32 ItemLevel;
	int32 PickupInstigatorhandle;
	Stack->StepCompiledIn(&InventoryOwner);
	Stack->StepCompiledIn(&ItemDefinition);
	Stack->StepCompiledIn(&ItemVariantGuid);
	Stack->StepCompiledIn(&NumberToGive);
	Stack->StepCompiledIn(&bNotifyPlayer);
	Stack->StepCompiledIn(&ItemLevel);
	Stack->StepCompiledIn(&PickupInstigatorhandle);

	if (ItemDefinition) {
		if (ItemDefinition->GetName().contains("WID_Launcher_Petrol")) return *Ret;
		printf("ItemDef: %s\n", ItemDefinition->GetFullName().c_str());
		*Ret = Inventory::AddItem(Util::Cast<AFortPlayerControllerAthena>(InventoryOwner.GetObjectRef()), ItemDefinition, NumberToGive);
	}
	return *Ret;
}

char ShouldAbilityRespondToEvent(__int64 a1, FFrame* a2, char* a3) {
	*a3 = true;
	return *a3;
}

void ServerHandlePickupInfo(AFortPlayerPawn* Pawn, AFortPickup* Pickup, FFortPickupRequestInfo& RequestInfo) {
	Pickup->PickupLocationData.bPlayPickupSound = RequestInfo.bPlayPickupSound;
	Pickup->PickupLocationData.FlyTime = 0.4f;
	Pickup->PickupLocationData.ItemOwner = Pawn;
	Pickup->PickupLocationData.PickupTarget = Pawn;
	Pickup->PickupLocationData.StartDirection = (FVector_NetQuantizeNormal)RequestInfo.Direction;
	Pickup->OnRep_PickupLocationData();
	Pickup->bPickedUp = true;
	Pickup->OnRep_bPickedUp();
}

void (*GivePickupToOG)(AFortPickup* thisPtr, IFortInventoryOwnerInterface* Interface, bool);
void GivePickupTo(AFortPickup* thisPtr, IFortInventoryOwnerInterface* Interface, bool DestroyAfterPickup) {
	UObject* (*GetObjectByAddress)(void* Interface) = decltype(GetObjectByAddress)(Interface->VTable[0x1]);
	AFortPlayerControllerAthena* Object = (AFortPlayerControllerAthena*)GetObjectByAddress(Interface);
	//printf("InterfaceObject: %s\n", Object->GetName().c_str());
	//printf("Ptr: %p\n", Interface->VTable);
	if (Object->IsA(AFortAthenaAIBotController::StaticClass())) return GivePickupToOG(thisPtr, Interface, DestroyAfterPickup);
	if (!Object->MyFortPawn) return GivePickupToOG(thisPtr, Interface, DestroyAfterPickup);

	FFortItemEntry* PickupEntryPtr = &thisPtr->PrimaryPickupItemEntry;
	FGuid ItemGuid = Object->MyFortPawn->CurrentWeapon->ItemEntryGuid;
	FGuid PreviousItemGuid = Object->MyFortPawn->PreviousWeapon ? Object->MyFortPawn->PreviousWeapon->ItemEntryGuid : FGuid();
	int32 PrimaryQuickbarItems = Inventory::GetNumQuickBarItems(Object);
	auto Instance = Inventory::GetItemFromGUID(Object, ItemGuid);
	auto PreviousInstance = Inventory::GetItemFromGUID(Object, PreviousItemGuid);
	UFortItemDefinition* ItemToDrop = Instance->ItemEntry.ItemDefinition;
	UFortItemDefinition* PreviousItem = PreviousInstance ? PreviousInstance->ItemEntry.ItemDefinition : nullptr;

	if (Inventory::GetQuickbar(PickupEntryPtr->ItemDefinition) != EFortQuickBars::Primary) {
		Inventory::AddItem(Object, PickupEntryPtr);
	}
	else if (PrimaryQuickbarItems < 5) {
		Inventory::AddItem(Object, PickupEntryPtr);
	}
	else {
		if (ItemToDrop && ItemToDrop->IsA(UFortWeaponMeleeItemDefinition::StaticClass())) {
			//printf("Should Not happen\n");
			//printf("PrevItem: %s\n", PreviousItem->GetName().c_str());
			FSpawnPickupData Data{};
			Data.ItemDefinition = PreviousItem;
			Data.Count = PreviousInstance ? PreviousInstance->ItemEntry.Count : 0;
			Data.Location = Object->MyFortPawn->K2_GetActorLocation();
			Data.PickupOwner = Object->MyFortPawn;
			Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Player;
			Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
			Inventory::SpawnPickup(Data);
			Inventory::RemoveItem(Object, PreviousItemGuid, PreviousInstance->ItemEntry.Count);
			Inventory::AddItem(Object, PickupEntryPtr);
		}
		else if (ItemToDrop) {
			FSpawnPickupData Data{};
			Data.ItemDefinition = ItemToDrop;
			Data.Count = Instance->ItemEntry.Count;
			Data.Location = Object->MyFortPawn->K2_GetActorLocation();
			Data.PickupOwner = Object->MyFortPawn;
			Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Player;
			Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
			Inventory::SpawnPickup(Data);
			Inventory::RemoveItem(Object, ItemGuid, Instance->ItemEntry.Count);
			Inventory::AddItem(Object, PickupEntryPtr);
		}
	}

	GivePickupToOG(thisPtr, Interface, DestroyAfterPickup);
}

void (*StartAircraftPhaseOG)(AFortGameModeAthena* GameMode, bool a2);
void StartAircraftPhase(AFortGameModeAthena* GameMode, bool a2) {
	StartAircraftPhaseOG(GameMode, a2);
	UAthenaAIServicePlayerBots* AIService = UAthenaAIBlueprintLibrary::GetAIServicePlayerBots(UWorld::GetWorld());
	if (AIService) {
		if (AIService->CachedGameState && AIService->CachedAIPopulationTracker) {
			for (auto& PlayerBot : AIService->PlayerBots) {
				if (PlayerBot.BotController) {
					auto Aircraft = AIService->CachedGameState->Aircrafts[0];
					//printf("Teleporting %s to aircraft\n", PlayerBot.BotController->GetName().c_str());
					Native::EnterAircraft(PlayerBot.BotController, Aircraft);
				}
			}
		}
	}
}

void SendCustomStatEvent(UFortQuestManager* QuestManager, const FDataTableRowHandle& ObjectiveStat, int32 Count, bool bForceFlush) {
	UE_LOG(LogFlipped, Log, "SendCustomStatEvent called with ObjectiveStat: %s, Count: %d, bForceFlush: %d", 
		ObjectiveStat.DataTable->GetName().c_str(), Count, bForceFlush);

}

void SendCustomStatEventDirect(UFortQuestManager* QuestManager, FName ObjectiveBackendName, UFortQuestItem* QuestItem, int32 Count, bool bForceFlush) {
	UE_LOG(LogFlipped, Log, "SendCustomStatEventDirect called with ObjectiveBackendName: %s, QuestItem: %s, Count: %d, bForceFlush: %d", 
		ObjectiveBackendName.ToString().c_str(), QuestItem ? QuestItem->GetName().c_str() : "None", Count, bForceFlush);
}

void SendComplexCustomStatEvent(UFortQuestManager* QuestManager, UObject* TargetObject, const FGameplayTagContainer& AdditionalSourceTags, const FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count) {
	UE_LOG(LogFlipped, Log, "SendComplexCustomStatEvent called with TargetObject: %s\n, AdditionalSourceTags: %s\n, TargetTags: %s\n, QuestActive: %d\n, QuestCompleted: %d\n, Count: %d\n", 
		"None", 
		AdditionalSourceTags.ToString().c_str(),
		TargetTags.ToString().c_str(),
		false, 
		false, 
		Count);
}

void SendCustomStatEventWithTags(UFortQuestManager* QuestManager, EFortQuestObjectiveStatEvent Type, const struct FGameplayTagContainer& AdditionalSourceTags, const struct FGameplayTagContainer& TargetTags, bool* QuestActive, bool* QuestCompleted, int32 Count) {
	UE_LOG(LogFlipped, Log, "SendCustomStatEventWithTags called with Type: %d, AdditionalSourceTags: %s, TargetTags: %s, QuestActive: %d, QuestCompleted: %d, Count: %d", 
		static_cast<int>(Type), 
		AdditionalSourceTags.ToString().c_str(), 
		TargetTags.ToString().c_str(), 
		false, 
		false, 
		Count);


}

void RemoveInventoryItem(IFortInventoryOwnerInterface* thisPtr, FGuid Guid, int a3, bool bForceRemoveFromQuickBars, bool bForceRemoval) {
	UE_LOG(LogFlipped, Log, "RemoveInventoryItem called with Count: %d, bForceRemoveFromQuickBars: %d, bForceRemoval: %d", 
		a3, bForceRemoveFromQuickBars, bForceRemoval);
	UObject* (*GetObjectByAddress)(void* Interface) = decltype(GetObjectByAddress)(thisPtr->VTable[0x1]);
	AFortPlayerControllerAthena* PlayerController = Util::Cast<AFortPlayerControllerAthena>(GetObjectByAddress(thisPtr));
	if (!PlayerController) {
		return;
	}

	Inventory::RemoveItem(PlayerController, Guid, a3);
}

void (*OnDamageServerOG)(ABuildingActor*, float, const struct FGameplayTagContainer&, const struct FVector&, const struct FHitResult&, class AController*, class AActor*, const struct FGameplayEffectContextHandle&);
void OnDamageServer(ABuildingSMActor* Actor, float Damage, const struct FGameplayTagContainer& DamageTags, const struct FVector& Momentum, const struct FHitResult& HitInfo, class AController* InstigatedBy, class AActor* DamageCauser, const struct FGameplayEffectContextHandle& EffectContext) {
	UE_LOG(LogFlipped, Log, "OnDamageServer called with Damage: %f, InstigatedBy: %s, DamageCauser: %s", 
		Damage, 
		InstigatedBy ? InstigatedBy->GetName().c_str() : "None",
		DamageCauser ? DamageCauser->GetName().c_str() : "None");
	OnDamageServerOG(Actor, Damage, DamageTags, Momentum, HitInfo, InstigatedBy, DamageCauser, EffectContext);

	if (!Actor || !InstigatedBy || !InstigatedBy->PlayerState || !InstigatedBy->Pawn || !DamageCauser) return;

	if (Actor->IsA(ABuildingSMActor::StaticClass()) 
		&& DamageCauser->IsA(AB_Athena_Pickaxe_Generic_C::StaticClass()) && InstigatedBy->IsA(AAthena_PlayerController_C::StaticClass()) && !Actor->bPlayerPlaced) {
		auto PC = (AFortPlayerControllerAthena*)InstigatedBy;
		if (PC->MyFortPawn && PC->MyFortPawn->CurrentWeapon) {
			float DataTableRowValue = 0.0f;
			UDataTableFunctionLibrary::EvaluateCurveTableRow(Actor->BuildingResourceAmountOverride.CurveTable, Actor->BuildingResourceAmountOverride.RowName, 0, nullptr, &DataTableRowValue, {});
			float ResourceAmount = round(DataTableRowValue / (Actor->GetMaxHealth() / Damage));
			PC->ClientReportDamagedResourceBuilding(Actor, Actor->ResourceType, ResourceAmount, false, Damage == 100.0f);
			Inventory::AddItem(PC, UFortKismetLibrary::K2_GetResourceItemDefinition(Actor->ResourceType), ResourceAmount);
		}
	}
}

void ServerClientIsReadyToRespawn(AFortPlayerControllerAthena* Controller) {
	printf(__FUNCTION__"\n");
	auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
	auto GameState = (AFortGameStateAthena*)GameMode->GameState;
	auto PlayerState = (AFortPlayerStateAthena*)Controller->PlayerState;

	if (GameState->IsRespawningAllowed(PlayerState))
	{
		FFortRespawnData* RespawnData = &PlayerState->RespawnData;

		FTransform Transform{};
		Transform.Translation = RespawnData->RespawnLocation;
		Transform.Rotation = UKismetMathLibrary::Conv_RotatorToQuaternion(RespawnData->RespawnRotation);
		Transform.Scale3D = FVector(1, 1, 1);

		AFortPlayerPawn* PlayerPawn = (AFortPlayerPawn*)GameMode->SpawnDefaultPawnAtTransform(Controller, Transform);

		if (!PlayerPawn)
			return;

		PlayerPawn->SetOwner(Controller);

		Controller->Possess(PlayerPawn);

		PlayerPawn->SetMaxHealth(100);
		PlayerPawn->SetHealth(100);
		PlayerPawn->SetMaxShield(100);
		PlayerPawn->SetShield(100);

		Controller->RespawnPlayerAfterDeath(true);

		RespawnData->bClientIsReady = true;
	}
}


// Parameters:
// bool                                    bShouldGrant                                           (BlueprintVisible, BlueprintReadOnly, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
// int32                                   GiveAmount                                             (BlueprintVisible, BlueprintReadOnly, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
// EFortResourceType                       GiveType                                               (BlueprintVisible, BlueprintReadOnly, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
// bool*                                   Success                                                (Parm, OutParm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash)
void GiveResourcesToPlayer(UGA_Creative_OnKillSiphon_C* Context, FFrame* Stack, void* Ret)
{
	printf(__FUNCTION__);
}