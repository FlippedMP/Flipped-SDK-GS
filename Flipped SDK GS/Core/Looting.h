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
        float RandomPoint = Misc::RandRange(0.0f, TotalWeight);

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

            TierGroupNameToUse = TierGroupName; // We are assuming that the TierGroupName is valid but idk
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

        static UDataTable* LootTierData = UObject::FindObject<UDataTable>("DataTable AthenaLootTierData_Client.AthenaLootTierData_Client");
        static UDataTable* LootPackages = UObject::FindObject<UDataTable>("DataTable AthenaLootPackages_Client.AthenaLootPackages_Client");
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

        printf("PackageName: %s\n",LootTierDataToUse->LootPackage.ToString().c_str());

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
        TArray<FFortLootPackageData*> FinalizedLootPackages;
        printf("NumLootPackageDrops: %f\n", LootTierDataToUse->NumLootPackageDrops);

        float NumLootPackageDrops = std::floor(LootTierDataToUse->NumLootPackageDrops);

        for (float i = 0; i < NumLootPackageDrops; i++) {
            FFortLootPackageData* LootPackage = CachedLootPackageData[i];

            if (!LootPackage) continue;

            if (LootPackage->LootPackageCall.ToString().empty()) {
                printf("No Empty LootPackageCall");
                FinalizedLootPackages.Add(LootPackage);
            }
            else {
                for (UDataTable* GameFeatureDataTable : GameFeatureLootPackageData)
                {
                    for (auto& [RowName, StructRow] : GameFeatureDataTable->RowMap) {
                        FFortLootPackageData* RowStruct = (FFortLootPackageData*)StructRow;
                        if (!RowStruct) continue;
                        if (RowStruct->LootPackageID.ToString() == LootPackage->LootPackageCall.ToString() && RowStruct->Weight != 0.0) {
                            printf("????4rfgrga");
                            FinalizedLootPackages.Add(RowStruct);
                        }
                    }
                }
            }

            printf("FinalizedLootPackages: %d \n", FinalizedLootPackages.Num());
                

            FFortLootPackageData* LootPackageCall = GetWeightedRow(FinalizedLootPackages);

            if (!LootPackageCall) continue;

            printf("idk: %s\n", LootPackageCall->LootPackageID.ToString().c_str());

            UFortItemDefinition* ItemDefinition = Native::StaticLoadObject<UFortItemDefinition>(LootPackageCall->ItemDefinition.ObjectID.AssetPathName.ToString());

            if (!ItemDefinition) PickLootDrops(WorldContextObject, OutLootToDrop, TierGroupName, WorldLevel, ForcedLootTier);

            printf("Adding %s to index %f\n", ItemDefinition->GetFullName().c_str(), i);

            FFortItemEntry LootDropEntry{};
            LootDropEntry.ItemDefinition = ItemDefinition;
            LootDropEntry.Count = LootPackageCall->Count;

            OutLootToDrop->Add(LootDropEntry);
        }

        CachedLootTierData.Free();
        CachedLootPackageData.Free();
        FinalizedLootPackages.Free();

        return OutLootToDrop->Num() > 0;
    }
}