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

    template<typename T>
    T* GetWeightedRow(const TArray<T*>& Rows)
    {
        if (Rows.Num() == 0)
            return nullptr;

        float TotalWeight = 0.0f;
        for (const T* Row : Rows)
        {
            if (Row)
            {
                TotalWeight += Row->Weight;
            }
        }

        if (TotalWeight <= 0.0f)
            return nullptr;

        std::cout << "TotalWeight: " << TotalWeight << std::endl;
        float RandomPoint = UKismetMathLibrary::RandomFloat() * TotalWeight;

        std::cout << "RandomPoint: " << RandomPoint << std::endl;

        float CumProb = 0.0f;

        for (T* Row : Rows)
        {
            if (!Row) continue;

            CumProb += Row->Weight;
            if (RandomPoint <= CumProb)
            {
                return Row;
            }
        }

        // Fallback
        return Rows[0];
    }

    void BuildLootTables(TArray<UDataTable*>* OutTierDataTables, TArray<UDataTable*>* OutLootPackageTables)
    {
        static UFortPlaylistAthena* LoadedPlaylist = Misc::CurrentPlaylist();
        if (!LoadedPlaylist)
            return;
        TSoftObjectPtr<UDataTable> LootTierTableSoftPtr = LoadedPlaylist->LootTierData;
        TSoftObjectPtr<UDataTable> LootPackagesSoftPtr = LoadedPlaylist->LootPackages;

        if (LootTierTableSoftPtr.IsValid() && LootPackagesSoftPtr.IsValid())
        {
            OutTierDataTables->Add(LootTierTableSoftPtr.NewGet());
            OutLootPackageTables->Add(LootPackagesSoftPtr.NewGet());
        }

        std::vector<UFortGameFeatureData*> GameFeatureDataArray = Misc::GetObjectsOfClass<UFortGameFeatureData>();

        for (UFortGameFeatureData* GameFeatureData : GameFeatureDataArray)
        {
            if (GameFeatureData->DefaultLootTableData.LootPackageData.IsValid() && GameFeatureData->DefaultLootTableData.LootTierData.IsValid())
            {
                UDataTable* DefaultLootTierData = GameFeatureData->DefaultLootTableData.LootTierData.NewGet();
                UDataTable* DefaultLootPackageData = GameFeatureData->DefaultLootTableData.LootPackageData.NewGet();

                if (DefaultLootTierData && DefaultLootPackageData) {
                    OutTierDataTables->Add(DefaultLootTierData);
                    OutLootPackageTables->Add(DefaultLootPackageData);
                }
            }

            if (GameFeatureData->PlaylistOverrideLootTableData.Num() > 0) {
                for (int i = 0; i < LoadedPlaylist->GameplayTagContainer.GameplayTags.Num(); i++) {
                    auto& Tag = LoadedPlaylist->GameplayTagContainer.GameplayTags[i];

                    for (auto& Value : GameFeatureData->PlaylistOverrideLootTableData)
                    {
                        auto CurrentOverrideTag = Value.First;

                        if (Tag.TagName == CurrentOverrideTag.TagName)
                        {
                            if (Value.Second.LootTierData.IsValid() && Value.Second.LootPackageData.IsValid())
                            {
                                UDataTable* OverrideLootTierData = Value.Second.LootTierData.NewGet();
                                UDataTable* OverrideLootPackageData = Value.Second.LootPackageData.NewGet();

                                OutTierDataTables->Add(OverrideLootTierData);
                                OutLootPackageTables->Add(OverrideLootPackageData);
                            }
                        }
                    }
                }
            }
        }
    }

    bool PickLootDrops(UWorld* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier)
    {
        if (!WorldContextObject)
            return false;
        
        static TArray<UDataTable*> LootTierDataTables;
        static TArray<UDataTable*> LootPackagesTables;

        
        if (LootTierDataTables.Num() <= 0 || LootPackagesTables.Num() <= 0) {
            BuildLootTables(&LootTierDataTables, &LootPackagesTables);
        }

        printf("LootTierDataTables: %d\n", LootTierDataTables.Num());
        printf("LootPackagesTables: %d\n", LootPackagesTables.Num());

        return OutLootToDrop->Num() > 0;
    }
}