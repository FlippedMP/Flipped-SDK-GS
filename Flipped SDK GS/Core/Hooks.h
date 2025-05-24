#pragma once
#include "../Other/Framework.h"
#include "./Inventory.h"
#include"./Looting.h"

void (*DispatchRequestOG)(__int64, __int64, int); void DispatchRequest(__int64 a1, __int64 a2, int a3) { return DispatchRequestOG(a1, a2, 3); }
void (*TickFlushOG)(UNetDriver*); void TickFlush(UNetDriver* Driver) { if (Driver && Driver->ReplicationDriver && Driver->ClientConnections.Num() > 0) Native::ServerReplicateActors(Driver->ReplicationDriver); TickFlushOG(Driver); }

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
			GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
			GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
			GameState->CurrentPlaylistInfo.MarkArrayDirty();
			GameState->OnRep_CurrentPlaylistInfo();

			GameState->CurrentPlaylistId = Playlist->PlaylistId;
			GameState->OnRep_CurrentPlaylistId();

			thisPtr->CurrentPlaylistId = Playlist->PlaylistId;
			thisPtr->CurrentPlaylistName = Playlist->PlaylistName;
			thisPtr->WarmupRequiredPlayerCount = 1;

			static FName SafeZoneDamageRowName = UKismetStringLibrary::Conv_StringToName(L"Default.SafeZone.Damage");
			GameState->AthenaGameDataResetRows.Add(SafeZoneDamageRowName);
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
			}

			GameState->DefaultRebootMachineHotfix = 1;


			SET_TITLE("Flipped 19.10 - Listening!");
		}

	}

	return thisPtr->NumPlayers >= thisPtr->WarmupRequiredPlayerCount;
}

APawn* SpawnDefaultPawnFor(AFortGameModeAthena* thisPtr, AFortPlayerControllerAthena* NewPlayer, AActor* StartSpot)
{
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
		if (AthenaGameDataTable) {

			auto& RowMap = AthenaGameDataTable->GetRowMap();
			for (auto& RowPair : RowMap) {
				FName RowName = RowPair.First;
				FSimpleCurve* Curve = reinterpret_cast<FSimpleCurve*>(RowPair.Second);

				if (RowName.ToString() == "Default.SafeZone.Damage") {
					FLIPPED_LOG("Apply SafeZoneDamage");
					for (auto& Key : Curve->Keys)
					{
						FSimpleCurveKey* KeyPtr = &Key;
						KeyPtr->Value = 0.f;
					}

					Curve->Keys.Add(FSimpleCurveKey(1.f, 0.01f));
				}
			}
		}
		else {
			FLIPPED_LOG("no AthenaGameDataTable");
		}

		static UDataTable* AthenaRangedWeapons = UObject::FindObject<UDataTable>("DataTable AthenaRangedWeapons.AthenaRangedWeapons");
		Misc::ApplyDataTablePatch(AthenaRangedWeapons);

		bFirst = true;
	}

	

	return NewPawn;
}

void ServerAcknowledgePossession(AFortPlayerControllerAthena* thisPtr, AFortPlayerPawnAthena* P)
{
	if (!thisPtr || !P)
		return;

	thisPtr->AcknowledgedPawn = P;

	AFortPlayerStateAthena* PlayerState = Util::Cast<AFortPlayerStateAthena>(thisPtr->PlayerState);
	if (!PlayerState)
		return;

	PlayerState->HeroType = thisPtr->CosmeticLoadoutPC.Character->HeroDefinition;
	UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(PlayerState);

	void* InterfaceAddress = Native::GetInterfaceAddress(P, IAbilitySystemInterface::StaticClass());

	if (!InterfaceAddress)
		return;

	TScriptInterface<IAbilitySystemInterface> Script;
	Script.ObjectPointer = P;
	Script.InterfacePointer = InterfaceAddress;

	UFortAbilitySet* DefaultAbilitySet = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer");

	if (DefaultAbilitySet)
		UFortKismetLibrary::EquipFortAbilitySet(Script, DefaultAbilitySet, nullptr);
	else
		FLIPPED_LOG("Invalid DefaultAbilitySet!");
}

void ServerExecuteInventoryItem(AFortPlayerControllerAthena* Controller, const FGuid& ItemGUID)
{
	if (!Controller || Controller->IsInAircraft())
		return;

	for (FFortItemEntry& ItemEntry : Controller->WorldInventory->Inventory.ReplicatedEntries)
	{
		if (ItemEntry.ItemGuid == ItemGUID)
		{
			if (!Controller->MyFortPawn) return;
			Controller->MyFortPawn->EquipWeaponDefinition(
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