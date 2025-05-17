#pragma once
#include "../Other/Framework.h"
#include "./Inventory.h"
#include"./Looting.h"

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

	AFortGameModeAthena* GameMode = Util::Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);

	for (FItemAndCount& _ : GameMode->StartingItems)
	{
		if (!_.Item || _.Count == 0 || _.Item->GetName().contains("Smart"))
			continue;

		Inventory::AddItem(NewPlayer, _.Item, _.Count);
	}


	if (FFortAthenaLoadout* CosmeticLoadoutPC = &NewPlayer->CosmeticLoadoutPC)
	{
		if (CosmeticLoadoutPC->Pickaxe)
		{
			Inventory::AddItem(NewPlayer, CosmeticLoadoutPC->Pickaxe->WeaponDefinition, 1);
		}
		else
		{
			FLIPPED_LOG("Failed to get pickaxe!");
		}
	}



	if (!NewPawn)
	{
		FLIPPED_LOG("Failed to spawn in new pawn!");
		return nullptr;
	}



	return NewPawn;
}

void ServerAcknowledgePossession(AFortPlayerControllerAthena* Controller, AFortPlayerPawnAthena* P)
{
	Controller->AcknowledgedPawn = P;

	void* InterfaceAddress = Native::GetInterfaceAddress(P, IAbilitySystemInterface::StaticClass());

	if (!InterfaceAddress)
	{
		std::cout << "Interface: " << InterfaceAddress << std::endl;
	}

	TScriptInterface<IAbilitySystemInterface> Script;
	Script.ObjectPointer = P;
	Script.InterfacePointer = InterfaceAddress;

	auto GAS = UObject::FindObject<UFortAbilitySet>("FortAbilitySet GAS_AthenaPlayer.GAS_AthenaPlayer");

	if (!GAS)
	{
		FLIPPED_LOG("No GAS");
	}

	auto Shit = UFortKismetLibrary::EquipFortAbilitySet(Script, GAS, nullptr);

	std::cout << "Hi: " << Shit.TargetAbilitySystemComponent.ObjectIndex << std::endl;

	Util::Cast<AFortPlayerState>(Controller->PlayerState)->HeroType = Controller->CosmeticLoadoutPC.Character->HeroDefinition;
	UFortKismetLibrary::UpdatePlayerCustomCharacterPartsVisualization(Util::Cast<AFortPlayerState>(Controller->PlayerState));
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

void (*DispatchRequestOG)(__int64, __int64, int);
void DispatchRequest(__int64 a1, __int64 a2, int a3)
{
	return DispatchRequestOG(a1, a2, 3);
}

void (*TickFlushOG)(UNetDriver*);
void TickFlush(UNetDriver* Driver)
{
	if (Driver && Driver->ReplicationDriver && Driver->ClientConnections.Num() > 0)
		Native::ServerReplicateActors(Driver->ReplicationDriver);

	TickFlushOG(Driver);
}