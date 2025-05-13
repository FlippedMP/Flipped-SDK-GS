#pragma once
#include "framework.h"
bool ReadyToStartMatch(AFortGameModeAthena* GameMode)
{
	if (!GameMode) return false;
	AFortGameStateAthena* GameState = (AFortGameStateAthena*)GameMode->GameState;
	if (!GameState || !GameState->MapInfo) return false;
	if (GameMode->CurrentPlaylistId == -1)
	{
		UFortPlaylistAthena* Playlist = UObject::FindObject<UFortPlaylistAthena>("FortPlaylistAthena Playlist_DefaultSolo.Playlist_DefaultSolo");
		GameState->CurrentPlaylistInfo.BasePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.OverridePlaylist = Playlist;
		GameState->CurrentPlaylistInfo.PlaylistReplicationKey++;
		GameState->CurrentPlaylistInfo.MarkArrayDirty();

		GameState->CurrentPlaylistId = Playlist->PlaylistId;

		GameMode->CurrentPlaylistId = Playlist->PlaylistId;
		GameMode->CurrentPlaylistName = Playlist->PlaylistName;
		GameMode->WarmupRequiredPlayerCount = 1;

		GameState->OnRep_CurrentPlaylistId();
		GameState->OnRep_CurrentPlaylistInfo();
	}

	static bool bLoadedProperly = false;
	if (!bLoadedProperly)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPlayerStartWarmup::StaticClass(), &Actors);

		int Num = Actors.Num();
		Actors.Free();
		if (Num == 0)
			return false;
		bLoadedProperly = true;
	}

	if (!GameMode->bWorldIsReady)
	{
		FName GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");
		void* WorldContext = Funcs::GetWorldFromContextObject(UEngine::GetEngine(), UWorld::GetWorld());
		SDK::UNetDriver* NetDriver = Funcs::CreateNetDriver_Local(UEngine::GetEngine(), WorldContext, GameNetDriver);

		UWorld::GetWorld()->NetDriver = NetDriver;

		if (UWorld::GetWorld()->NetDriver)
		{
			UWorld::GetWorld()->NetDriver->NetDriverName = GameNetDriver;
			UWorld::GetWorld()->NetDriver->World = UWorld::GetWorld();

			SDK::FURL URL{};
			URL.Port = 7777;

			Funcs::InitListen(UWorld::GetWorld()->NetDriver, UWorld::GetWorld(), URL, false, {});
			Funcs::SetWorld(UWorld::GetWorld()->NetDriver, UWorld::GetWorld());


			for (int i = 0; i < UWorld::GetWorld()->LevelCollections.Num(); i++)
			{
				UWorld::GetWorld()->LevelCollections[i].NetDriver = UWorld::GetWorld()->NetDriver;
			}
		}

		GameMode->bWorldIsReady = true;
		SetConsoleTitleA("Accepting Connections");
	}
	return GameMode->NumPlayers >= GameMode->WarmupRequiredPlayerCount;
}

AActor* SpawnDefaultPawnFor(AFortGameModeAthena* GameMode, AFortPlayerController* PC, AActor* StartSpot)
{
	printf(__FUNCTION__);
	return GameMode->SpawnDefaultPawnAtTransform(PC, StartSpot->GetTransform());
}

void (*TickFlush)(UNetDriver* Driver, float DeltaSeconds);
void TickFlushHook(UNetDriver* Driver, float DeltaSeconds)
{
	if (Driver->ReplicationDriver)
	{
		Funcs::ServerReplicateActors(Driver->ReplicationDriver, DeltaSeconds);
	}

	return TickFlush(Driver, DeltaSeconds);
}