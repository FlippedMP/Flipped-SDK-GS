#pragma once
#include "../Other/Framework.h"
#include "./Inventory.h"

struct FSpawnPickupData
{
    UFortItemDefinition* ItemDefinition;
    FVector Location;
    FRotator Rotation = FRotator(0, 0, 0);
    bool bRandomRotation = Rotation == FRotator(0, 0, 0);
    int Count = 1;
    int LoadedAmmo = -1;
    AFortPawn* ItemOwner = nullptr;
    EFortPickupSourceTypeFlag FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Tossed;
    EFortPickupSpawnSource FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
};

namespace Looting
{
    int32 GetClipSize(UFortItemDefinition* Definition)
    {
        if (Definition->IsA(UFortWeaponRangedItemDefinition::StaticClass()))
        {
            UFortWeaponRangedItemDefinition* WeaponDefinition = (UFortWeaponRangedItemDefinition*)Definition;
            if (!WeaponDefinition)
                return 0;

            TMap<FName, uint8*>& RowMap = WeaponDefinition->WeaponStatHandle.DataTable->RowMap;
            if (!&RowMap)
                return 0;

            for (int i = 0; i < RowMap.Num(); i++)
            {
                if (RowMap[i].First == WeaponDefinition->WeaponStatHandle.RowName)
                    return ((FFortBaseWeaponStats*)RowMap[i].Second)->ClipSize;
            }
        }

        return 0;
    }

    AFortPickupAthena* SpawnPickup(const FSpawnPickupData& SpawnPickupData)
    {
        UFortItemDefinition* ItemDefinition = SpawnPickupData.ItemDefinition;
        FVector Location = SpawnPickupData.Location;
        FRotator Rotation = SpawnPickupData.Rotation;
        bool bRandomRotation = SpawnPickupData.bRandomRotation;
        int Count = SpawnPickupData.Count;
        int LoadedAmmo = SpawnPickupData.LoadedAmmo;
        AFortPawn* ItemOwner = SpawnPickupData.ItemOwner;
        EFortPickupSourceTypeFlag FortPickupSourceTypeFlag = SpawnPickupData.FortPickupSourceTypeFlag;
        EFortPickupSpawnSource FortPickupSpawnSource = SpawnPickupData.FortPickupSpawnSource;

        if (!ItemDefinition || Count == 0)
            return nullptr;

        if (AFortPickupAthena* NewPickup = Misc::SpawnActor<AFortPickupAthena>(Location, Rotation))
        {
            NewPickup->PawnWhoDroppedPickup = ItemOwner;
            NewPickup->PrimaryPickupItemEntry.ItemDefinition = ItemDefinition;
            NewPickup->PrimaryPickupItemEntry.Count = Count;

            if (LoadedAmmo == -1)
                LoadedAmmo = Looting::GetClipSize(ItemDefinition);

            NewPickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
            NewPickup->OnRep_PrimaryPickupItemEntry();

            NewPickup->bRandomRotation = bRandomRotation;

            if (NewPickup->MovementComponent)
            {
                NewPickup->SetActorEnableCollision(true);
                NewPickup->MovementComponent->bShouldBounce = true;
                NewPickup->MovementComponent->Bounciness += 10.f;

                NewPickup->SetReplicateMovement(true);
                NewPickup->bReplicateMovement = true;
                NewPickup->OnRep_ReplicateMovement();
            }

            FVector FinalLocation = Location;

            if (FortPickupSourceTypeFlag == EFortPickupSourceTypeFlag::Container)
            {
                FinalLocation = Location + (NewPickup->GetActorForwardVector() * 200.f);
                FinalLocation += Misc::GetRandomLocationInCircle(Location, 300.f);

                NewPickup->bTossedFromContainer = true;
                NewPickup->OnRep_TossedFromContainer();
            }

            if (FortPickupSpawnSource == EFortPickupSpawnSource::PlayerElimination)
                FinalLocation = Misc::GetRandomLocationInCircle(Location, 1700.f);

            NewPickup->TossPickup(FinalLocation, ItemOwner, 0, true, false, FortPickupSourceTypeFlag, FortPickupSpawnSource);

            return NewPickup;
        }

        return nullptr;
    }
}