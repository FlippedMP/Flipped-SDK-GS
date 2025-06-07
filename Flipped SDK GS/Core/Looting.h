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

    void PickLootDropsFromPackage(const TArray<UDataTable*>& LootPackagesTables, TArray<FFortItemEntry>* OutItemEntries, FName LootPackageName)
    {
        TArray<FFortLootPackageData*> CachedLootPackageData;
        for (int i = 0; i < LootPackagesTables.Num(); i++)
        {
            auto& LootPackageTable = LootPackagesTables[i];

            if (!LootPackageTable) continue;

            printf("[%d] LP %s\n", i, LootPackageTable->GetFullName().c_str());

            for (TPair<FName, uint8*> Pair : LootPackageTable->RowMap)
            {
                FFortLootPackageData* LootPackage = reinterpret_cast<FFortLootPackageData*>(Pair.Second);

                if (LootPackage->LootPackageID != LootPackageName)
                    continue;

                if (LootPackage->Weight == 0)
                    continue;

                CachedLootPackageData.Add(LootPackage);
            }
        }

        FFortLootPackageData* LootPackage = GetWeightedRow(CachedLootPackageData);
        if (!LootPackage)
            return;

        if (LootPackage->LootPackageCall.Num() > 1)
        {
            if (LootPackage->Count > 0)
            {
                for (int i = 0; i < LootPackage->Count; i++)
                    PickLootDropsFromPackage(LootPackagesTables, OutItemEntries, UKismetStringLibrary::Conv_StringToName(LootPackage->LootPackageCall));
            }
            return;
        }

        UFortItemDefinition* Definition = Native::StaticLoadObject<UFortItemDefinition>(
            LootPackage->ItemDefinition.ObjectID.AssetPathName.ToString());

        printf("Def: %s\n", Definition->GetFullName().c_str());

        FFortItemEntry Entry{};
        Entry.ItemDefinition = Definition;
        Entry.Count = LootPackage->Count;
        OutItemEntries->Add(Entry);
    }

    // Parameters:
// class UObject*                          WorldContextObject                                     (Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// TArray<struct FFortItemEntry>*          OutLootToDrop                                          (Parm, OutParm, ZeroConstructor, NativeAccessSpecifierPublic)
// const class FName                       TierGroupName                                          (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// const int32                             WorldLevel                                             (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// const int32                             ForcedLootTier                                         (ConstParm, Parm, ZeroConstructor, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
// bool                                    ReturnValue                                            (Parm, OutParm, ZeroConstructor, ReturnParm, IsPlainOldData, NoDestructor, HasGetValueTypeHash, NativeAccessSpecifierPublic)
    bool PickLootDrops(UWorld* WorldContextObject, TArray<FFortItemEntry>* OutLootToDrop, const FName TierGroupName, const int32 WorldLevel, const int32 ForcedLootTier)
    {
        if (!WorldContextObject)
            return false;
        
        static TArray<UDataTable*> LootTierDataTables;
        static TArray<UDataTable*> LootPackagesTables;

        static int IncrementatedNum = 0;

        if (IncrementatedNum == 0) {
            IncrementatedNum++;

            if (UFortPlaylistAthena* Playlist = Misc::CurrentPlaylist()) {
                auto& LootTierDataSoftPtr = Playlist->LootTierData;
                auto& LootPackagesSoftPtr = Playlist->LootPackages;
                if (LootTierDataSoftPtr.ObjectID.IsValid() && LootPackagesSoftPtr.ObjectID.IsValid()) {
                    std::string LootTierDataStr = LootTierDataSoftPtr.ObjectID.AssetPathName.ToString();
                    std::string LootPackagesStr = LootPackagesSoftPtr.ObjectID.AssetPathName.ToString();

                    UDataTable* StrongLootTierData = nullptr;
                    UDataTable* StrongLootPackage = nullptr;

                    StrongLootTierData = Native::StaticLoadObject<UCompositeDataTable>(LootTierDataStr);
                    StrongLootPackage = Native::StaticLoadObject<UCompositeDataTable>(LootPackagesStr);

                    if (StrongLootTierData && StrongLootPackage)
                    {
                        LootTierDataTables.Add(StrongLootTierData);
                        LootPackagesTables.Add(StrongLootPackage);
                    }
                }
                else if (Playlist->bIsDefaultPlaylist)
                {
                    LootTierDataTables.Add(Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
                    LootPackagesTables.Add(Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));
                }
            }
            LootTierDataTables.Add(Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client"));
            LootPackagesTables.Add(Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client"));

            auto Shit = Misc::GetObjectsOfClass<UFortGameFeatureData>(UFortGameFeatureData::StaticClass());

            for (auto Dih : Shit)
            {

            }
        }

        TArray<FFortLootTierData*> CachedLootTierData;
        TArray<FFortLootPackageData*> CachedLootPackageData;

        for (int i = 0; i < LootTierDataTables.Num(); i++) {
            auto& LootTierDataTable = LootTierDataTables[i];

            if (!LootTierDataTable) continue;

            printf("[%d] LTD: %s\n", i, LootTierDataTable->GetFullName().c_str());

            for (TPair<FName, uint8*> Pair : LootTierDataTable->RowMap)
            {
                FFortLootTierData* TierData = reinterpret_cast<FFortLootTierData*>(Pair.Second);
                if (TierGroupName == TierData->TierGroup && TierData->Weight != 0) {
                    CachedLootTierData.Add(TierData);
                }
            }
        }

        FFortLootTierData* RowStruct = GetWeightedRow(CachedLootTierData);

        if (!RowStruct)
            return false;

        int NumLootDrops = 0;

        if (RowStruct->LootPackageCategoryMinArray.Num())
        {
            for (int i = 0; i < RowStruct->LootPackageCategoryMinArray.Num(); i++)
            {
                if (RowStruct->LootPackageCategoryMinArray[i] > 0)
                    NumLootDrops += RowStruct->LootPackageCategoryMinArray[i];
            }
        }

        if (NumLootDrops > 0)
        {
            for (int i = 0; i < NumLootDrops; i++)
            {
                if (i >= RowStruct->LootPackageCategoryMinArray.Num())
                    break;

                for (int j = 0; j < RowStruct->LootPackageCategoryMinArray[i]; j++)
                {
                    if (RowStruct->LootPackageCategoryMinArray[i] < 1)
                        break;
                    PickLootDropsFromPackage(LootPackagesTables, OutLootToDrop, RowStruct->LootPackage);
                }
            }
        }


        return OutLootToDrop->Num() > 0;
    }
}