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

	AFortGameModeAthena* GameMode = Util::Cast<AFortGameModeAthena>(GetWorld()->AuthorityGameMode);
	for (FItemAndCount& _ : GameMode->StartingItems)
	{
		if (!_.Item || _.Count == 0)
			continue;

		Inventory::AddItem(Controller, _.Item, _.Count);
	}

	P->ServerChoosePart(EFortCustomPartType::Head,
		Native::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Heads/F_Med_Head1.F_Med_Head1"));

	P->ServerChoosePart(EFortCustomPartType::Body,
		Native::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Female/Medium/Bodies/F_Med_Soldier_01.F_Med_Soldier_01"));

	P->ServerChoosePart(EFortCustomPartType::Backpack,
		Native::FindObject<UCustomCharacterPart>("/Game/Characters/CharacterParts/Backpacks/NoBackpack.NoBackpack"));


	if (FFortAthenaLoadout* CosmeticLoadoutPC = &Controller->CosmeticLoadoutPC)
	{
		if (CosmeticLoadoutPC->Pickaxe)
		{
			Inventory::AddItem(Controller, CosmeticLoadoutPC->Pickaxe->WeaponDefinition, 1);
		}
		else
		{
			FLIPPED_LOG("Failed to get pickaxe!");
		}
	}
}

void ServerExecuteInventoryItem(AFortPlayerControllerAthena* Controller, const FGuid& ItemGUID)
{
	if (!Controller)
		return;

	for (UFortWorldItem*& Item : Controller->WorldInventory->Inventory.ItemInstances)
	{
		if (Item->ItemEntry.ItemGuid == ItemGUID)
		{
			Controller->MyFortPawn->EquipWeaponDefinition(
				Util::Cast<UFortWeaponItemDefinition>(Item->ItemEntry.ItemDefinition),
				ItemGUID,
				Item->ItemEntry.TrackerGuid,
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