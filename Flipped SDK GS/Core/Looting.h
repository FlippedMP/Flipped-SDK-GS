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

        /*
        TSoftObjectPtr<UDataTable> LootTierTableSoftPtr = LoadedPlaylist->LootTierData;
        TSoftObjectPtr<UDataTable> LootPackagesSoftPtr = LoadedPlaylist->LootPackages;

        if (LootTierTableSoftPtr.IsValid() && LootPackagesSoftPtr.IsValid() && !LoadedPlaylist->bIsDefaultPlaylist)
        {
            UDataTable* LootTierTablePlaylistTable = LootTierTableSoftPtr.NewGet();
            UDataTable* LootPackagesPlaylistTable = LootPackagesSoftPtr.NewGet();
            if (LootTierTablePlaylistTable && LootPackagesPlaylistTable)
            {
                OutTierDataTables->Add(LootTierTablePlaylistTable);
                OutLootPackageTables->Add(LootPackagesPlaylistTable);
            }
        }
        else
        {
            
        } */

        UDataTable* BaseTierDataTable = Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
        UDataTable* BasePackagesDataTable = Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
        if (BaseTierDataTable && BasePackagesDataTable)
        {
            OutTierDataTables->Add(BaseTierDataTable);
            OutLootPackageTables->Add(BasePackagesDataTable);
        }

        /*

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

                                if (OverrideLootPackageData && OverrideLootTierData)
                                {
                                    OutTierDataTables->Add(OverrideLootTierData);
                                    OutLootPackageTables->Add(OverrideLootPackageData);
                                }
                            }
                        }
                    }
                }
            }
        } */
    }

    void PickLootDropsFromLootPackage(const TArray<UDataTable*>& LootPackageTables, TArray<FFortLootPackageData*>& CachedLootPackageData, const FName& LootPackageName, TArray<FFortItemEntry>*& OutEntries, int LootPackageCategory = -1) {
        if (!OutEntries)
            return;

        for (UDataTable* LootPackageDataTable : LootPackageTables) {
            if (!LootPackageDataTable) continue;
            for (TPair<FName, uint8*> RowPair : LootPackageDataTable->RowMap) {
                FFortLootPackageData* RowData = reinterpret_cast<FFortLootPackageData*>(RowPair.Second);
                if (RowData && RowData->LootPackageID == LootPackageName && RowData->Weight != 0) {
                    CachedLootPackageData.Add(RowData);
                }
            }
        }

        //printf("CachedLootPackageData: %d\n", CachedLootPackageData.Num());

        FFortLootPackageData* LootPackageRow = GetWeightedRow(CachedLootPackageData);

        if (!LootPackageRow)
            return;

        if (LootPackageRow->LootPackageCall.Num() > 1)
        {
            if (LootPackageRow->Count > 0) {
                int i = 0;
                while (i < LootPackageRow->Count) {
                    PickLootDropsFromLootPackage(LootPackageTables, CachedLootPackageData, UKismetStringLibrary::Conv_StringToName(LootPackageRow->LootPackageCall), OutEntries, -1);

                    i++;
                }
            }

            return;
        }

        UFortItemDefinition* ItemDefinition = LootPackageRow->ItemDefinition.NewGet();
        if (!ItemDefinition) return;

        FFortItemEntry LootDrop{};
        LootDrop.ItemDefinition = ItemDefinition;
        LootDrop.Count = LootPackageRow->Count;

        if (auto WeaponFortDefinition = Util::Cast<UFortWeaponRangedItemDefinition>(ItemDefinition)) {
            FFortItemEntry AmmoLootDrop{};
            AmmoLootDrop.ItemDefinition = WeaponFortDefinition->GetAmmoWorldItemDefinition_BP();
            AmmoLootDrop.Count = WeaponFortDefinition->GetAmmoWorldItemDefinition_BP()->DropCount;
            OutEntries->Add(AmmoLootDrop);
        }

        OutEntries->Add(LootDrop);
    }

    float GetAmountOfLootPackagesToDrop(FFortLootTierData* LootTierData, int OriginalNumLootDrops)
    {
        if (LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryWeightArray.Num()
            || LootTierData->LootPackageCategoryMinArray.Num() != LootTierData->LootPackageCategoryMaxArray.Num()
            )
            return 0;

        float MinimumLootDrops = 0;

        if (LootTierData->LootPackageCategoryMinArray.Num() > 0)
        {
            for (int i = 0; i < LootTierData->LootPackageCategoryMinArray.Num(); i++)
            {
                MinimumLootDrops += LootTierData->LootPackageCategoryMinArray[i];
            }
        }

        int SumLootPackageCategoryWeightArray = 0;

        if (LootTierData->LootPackageCategoryMinArray.Num() > 0)
        {
            for (int i = 0; i < LootTierData->LootPackageCategoryMinArray.Num(); ++i)
            {

                if (LootTierData->LootPackageCategoryMinArray[i] > 0)
                {
                    auto LootPackageCategoryMaxArrayIt = LootTierData->LootPackageCategoryMaxArray[i];

                    float IDK = 0;

                    if (LootPackageCategoryMaxArrayIt < 0 || IDK < LootPackageCategoryMaxArrayIt)
                    {
                        SumLootPackageCategoryWeightArray += LootTierData->LootPackageCategoryMinArray[i];
                    }
                }
            }
        }

        while (SumLootPackageCategoryWeightArray > 0)
        {
            float v29 = (float)rand() * 0.000030518509f;

            float v35 = (int)(float)((float)((float)((float)SumLootPackageCategoryWeightArray * v29)
                + (float)((float)SumLootPackageCategoryWeightArray * v29))
                + 0.5f) >> 1;

            MinimumLootDrops++;

            if (MinimumLootDrops >= OriginalNumLootDrops)
                return MinimumLootDrops;

            SumLootPackageCategoryWeightArray--;
        }

        return MinimumLootDrops;
    }

    TArray<UDataTable*> LootTierDataTables;
    TArray<UDataTable*> LootPackagesTables;
    bool PickLootDrops(UWorld* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier, bool bForceUpdateTables = false)
    {
        if (!WorldContextObject)
            return false;
        


        if (bForceUpdateTables || (LootTierDataTables.Num() <= 0 && LootPackagesTables.Num() <= 0))
        {
            LootTierDataTables.Free();
            LootPackagesTables.Free();
            BuildLootTables(&LootTierDataTables, &LootPackagesTables);
        }


        printf("TierGroupName: %s\n", TierGroupName.ToString().c_str());

        printf("LootTierDataTables: %d\n", LootTierDataTables.Num());
        printf("LootPackagesTables: %d\n", LootPackagesTables.Num());

        TArray<FFortLootTierData*> CachedLootTierData;
        TArray<FFortLootPackageData*> CachedLootPackageData;

        for (UDataTable* LootTierDataTable : LootTierDataTables)
        {
            if (!LootTierDataTable) continue;
            //printf("LTTable: %s\n", LootTierDataTable->GetFullName().c_str());
            for (TPair<FName, uint8*> RowPair : LootTierDataTable->RowMap) {
                FFortLootTierData* RowData = reinterpret_cast<FFortLootTierData*>(RowPair.Second);
                if (RowData && RowData->TierGroup == TierGroupName && RowData->Weight != 0) {
                    CachedLootTierData.Add(RowData);
                }
            }
        }

        FFortLootTierData* LootTierDataRow = GetWeightedRow(CachedLootTierData);

        if (!LootTierDataRow)
            return false;

        float NumberLootDrops = 0;

        if (LootTierDataRow->NumLootPackageDrops > 0)
        {
            if (LootTierDataRow->NumLootPackageDrops < 1)
                NumberLootDrops = 1;
            else
            {
                NumberLootDrops = (int)(float)((float)(LootTierDataRow->NumLootPackageDrops + LootTierDataRow->NumLootPackageDrops) - 0.5f) >> 1;
                float v20 = LootTierDataRow->NumLootPackageDrops - NumberLootDrops;
                if (v20 > 0.0000099999997f)
                {
                    NumberLootDrops += v20 >= (rand() * 0.000030518509f);
                }
            }
        }

        float AmountOfLootPackageDrops = GetAmountOfLootPackagesToDrop(LootTierDataRow, NumberLootDrops);

        printf("AmountOfLootPackageDrops: %f", AmountOfLootPackageDrops);

        if (AmountOfLootPackageDrops > 0) {
            for (int i = 0; i < AmountOfLootPackageDrops; i++)
            {
                if (i >= LootTierDataRow->LootPackageCategoryMinArray.Num())
                    break;

                for (int j = 0; j < LootTierDataRow->LootPackageCategoryMinArray[i]; j++)
                {
                    if (LootTierDataRow->LootPackageCategoryMinArray[i] < 1)
                        break;

                    printf("PickingLootDropFromLootPackage\n");
                    PickLootDropsFromLootPackage(LootPackagesTables, CachedLootPackageData, LootTierDataRow->LootPackage, OutLootToDrop, i);
                }
            }
        }

        CachedLootPackageData.Free();
        CachedLootTierData.Free();


        return OutLootToDrop->Num() > 0;
    }
}