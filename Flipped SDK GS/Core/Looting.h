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

    FFortLootTierData* GetWeightedRow(const TArray<FFortLootTierData*>& Rows)
    {
        if (Rows.Num() == 0)
            return nullptr;

        float TotalWeight = 0.0f;
        for (const FFortLootTierData* Row : Rows)
        {
            if (Row)
            {
                TotalWeight += Row->Weight;
            }
        }

        if (TotalWeight <= 0.0f)
            return nullptr;

        float RandomPoint = UKismetMathLibrary::RandomFloatInRange(0.0f, TotalWeight);

        for (FFortLootTierData* Row : Rows)
        {
            if (!Row) continue;

            RandomPoint -= Row->Weight;
            if (RandomPoint <= 0.0f)
            {
                return Row;
            }
        }

        // Fallback
        return Rows[Rows.Num()];
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
        AFortGameStateAthena* GameState = Util::Cast<AFortGameStateAthena>(WorldContextObject->GameState);
        AFortGameModeAthena* GameMode = Util::Cast<AFortGameModeAthena>(WorldContextObject->AuthorityGameMode);
        if (!GameMode || !GameState)
            return false;
        TArray<UDataTable*> GameFeatureLootPackageData;
        TArray<UDataTable*> GameFeatureLootTierData;
        GameFeatureLootPackageData.Reserve(1);
        GameFeatureLootTierData.Remove(1);

        printf("Num: %d", GameMode->GameFeatureLootPackageData.Num());
        printf("Num2: %d", GameMode->GameFeatureLootPackageData.Num());

        FName TierGroupNameToUse;
        for (auto& [SupportTierGroup, Redirect] : GameMode->RedirectAthenaLootTierGroups)
        {
            if (SupportTierGroup.ToString() == TierGroupName.ToString()) {
                TierGroupNameToUse = Redirect;
                break;
            }

            TierGroupNameToUse = TierGroupName;
        }

        if (TierGroupNameToUse.ToString() == "None") {
            FLIPPED_LOG("returning false because bad TierGroup");
            return false;
        }

        printf("TierGroupName: %s \n", TierGroupNameToUse.ToString().c_str());

        for (auto& GameFeatureLootPackageDataWeakObject : GameMode->GameFeatureLootPackageData)
        {
            UDataTable* GameFeatureDataTable = Native::StaticLoadObject<UDataTable>(
                GameFeatureLootPackageDataWeakObject.ObjectID.AssetPathName.ToString());
            GameFeatureLootPackageData.Add(GameFeatureDataTable);
        }
        for (auto& GameFeatureLootTierDataWeakObject : GameMode->GameFeatureLootTierData)
        {
            UDataTable* GameFeatureDataTable = Native::StaticLoadObject<UDataTable>(
                GameFeatureLootTierDataWeakObject.ObjectID.AssetPathName.ToString());
            GameFeatureLootTierData.Add(GameFeatureDataTable);
        }

        UDataTable* LootTierData = UObject::FindObject<UDataTable>("DataTable AthenaLootTierData_Client.AthenaLootTierData_Client");
        UDataTable* LootPackages = UObject::FindObject<UDataTable>("DataTable AthenaLootPackages_Client.AthenaLootPackages_Client");
        GameFeatureLootTierData.Add(LootTierData);
        GameFeatureLootPackageData.Add(LootPackages);

        TArray<FFortLootTierData*> CachedLootTierData;

        printf("eadfaefd: %d\n", GameFeatureLootPackageData.Num());
        printf("dih: %d\n", GameFeatureLootTierData.Num());

        for (UDataTable* GameFeatureDataTable : GameFeatureLootTierData)
        {
            for (auto& [RowName, StructRow] : GameFeatureDataTable->RowMap) {
                FFortLootTierData* RowStruct = (FFortLootTierData*)StructRow;
                if (!RowStruct) continue;
                if (RowStruct->TierGroup == TierGroupNameToUse && RowStruct->Weight != 0.0)
                    CachedLootTierData.Add(RowStruct);
            }
        }

        printf("CachedLootTierData: %d\n", CachedLootTierData.Num());

        FFortLootTierData* LootTierDataToUse = GetWeightedRow(CachedLootTierData);
        if (!LootTierDataToUse) return false;

        printf("PackageName: %s",LootTierDataToUse->LootPackage.ToString().c_str());

        TArray<FFortLootPackageData*> CachedLootPackageData;

        for (UDataTable* GameFeatureDataTable : GameFeatureLootPackageData)
        {
            for (auto& [RowName, StructRow] : GameFeatureDataTable->RowMap) {
                FFortLootPackageData* RowStruct = (FFortLootPackageData*)StructRow;
                if (!RowStruct) continue;
                if (RowStruct->LootPackageID == LootTierDataToUse->LootPackage && RowStruct->Weight != 0.0)
                    CachedLootPackageData.Add(RowStruct);
            }
        }

        printf("CachedLootPackageData: %d\n", CachedLootPackageData.Num());


    }
}