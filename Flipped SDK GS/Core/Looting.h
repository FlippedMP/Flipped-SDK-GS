#pragma once
#include "../Other/Framework.h"
#include <numeric>
#include "Inventory.h"

struct FSpawnPickupData
{
    UFortItemDefinition* ItemDefinition;
    FVector Location;
    FRotator Rotation = FRotator(0, 0, 0);
    bool bRandomRotation = Rotation == FRotator(0, 0, 0);
    int Count = 1;
    int LoadedAmmo = -1;
    AFortPawn* PickupOwner = nullptr;
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
        AFortPawn* PickupOwner = SpawnPickupData.PickupOwner;
        EFortPickupSourceTypeFlag FortPickupSourceTypeFlag = SpawnPickupData.FortPickupSourceTypeFlag;
        EFortPickupSpawnSource FortPickupSpawnSource = SpawnPickupData.FortPickupSpawnSource;

        if (!ItemDefinition || Count == 0)
            return nullptr;

        if (AFortPickupAthena* NewPickup = Misc::SpawnActor<AFortPickupAthena>(Location, Rotation))
        {
            NewPickup->PawnWhoDroppedPickup = PickupOwner;
            NewPickup->PrimaryPickupItemEntry.ItemDefinition = ItemDefinition;
            NewPickup->PrimaryPickupItemEntry.Count = Count;

            if (LoadedAmmo == -1)
                LoadedAmmo = Looting::GetClipSize(ItemDefinition);

            NewPickup->PrimaryPickupItemEntry.LoadedAmmo = LoadedAmmo;
            NewPickup->OnRep_PrimaryPickupItemEntry();

            NewPickup->bRandomRotation = bRandomRotation;

            if (!NewPickup->PickupLocationData.CombineTarget)
            {
                NewPickup->TossPickup(Location, PickupOwner, 0, true, false, FortPickupSourceTypeFlag, FortPickupSpawnSource);
            }

            return NewPickup;
        }

        return nullptr;
    }

    template<typename T>
    T* GetWeightedRow(const std::vector<T*>& Rows)
    {
        if (Rows.size() == 0)
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


    std::vector<UDataTable*> LootTierDataTables;
    std::vector<UDataTable*> LootPackagesTables;

    template<typename T>
    static T* PickWeighted(std::vector<T*>& Map, float (*RandFunc)(float)) {
        float TotalWeight = std::accumulate(Map.begin(), Map.end(), 0.0f, [&](float acc, const T* p) { return acc + p->Weight; });
        float RandomNumber = RandFunc(TotalWeight);

        for (auto& Element : Map)
        {
            float Weight = Element->Weight;

            if (Weight == 0)
                continue;

            if (RandomNumber <= Weight) return Element;

            RandomNumber -= Weight;
        }

        return nullptr;
    }

    void SetupLDSForPackage(TArray<FFortItemEntry*>* LootDrops, SDK::FName Package, int i, int WorldLevel = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel) {
        std::vector<FFortLootPackageData*> LPGroupsAll;

        auto LootPackages = Native::StaticLoadObject<UDataTable>(((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist->LootPackages.ObjectID.AssetPathName.ToString().c_str());
        if (!LootPackages) LootPackages = Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootPackages_Client.AthenaLootPackages_Client");
        if (!LootPackages) return;

        static UFortPlaylistAthena* LoadedPlaylist = Misc::GetCurrentPlaylist();
        if (!LoadedPlaylist)
            return;

        LootPackagesTables.push_back(LootPackages);

        for (UDataTable* LootPackageTable : LootPackagesTables) {
            if (!LootPackageTable) continue;
            for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>) LootPackageTable->RowMap) {
                LPGroupsAll.push_back(Val);
            }
        }

        if (auto CompositeTable = Util::Cast<UCompositeDataTable>(LootPackages)) {
            for (auto& PT : CompositeTable->ParentTables) {
                for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>) PT->RowMap) {
                    LPGroupsAll.push_back(Val);
                }
            }
        }

        std::vector<FFortLootPackageData*> LPGroups;
        for (auto const& Val : LPGroupsAll) {
            if (Val->LootPackageID == Package && (i != -1 ? Val->LootPackageCategory == i : true) && (WorldLevel >= 0 ? (
                (Val->MaxWorldLevel >= 0 ? WorldLevel <= Val->MaxWorldLevel : true) &&
                (Val->MinWorldLevel >= 0 ? WorldLevel >= Val->MinWorldLevel : true)
                ) : true)) LPGroups.push_back(Val);
        }
        if (LPGroups.size() == 0) return;

        auto LootPackage = GetWeightedRow(LPGroups);
        if (!LootPackage) return;
        if (LootPackage->LootPackageCall.Num() > 1)
        {
            for (int i = 0; i < LootPackage->Count; i++) {
                SetupLDSForPackage(LootDrops, UKismetStringLibrary::Conv_StringToName(LootPackage->LootPackageCall), 0);
            }

            return;
        }

        auto ItemDefinition = Util::Cast<UFortWorldItemDefinition>(LootPackage->ItemDefinition.NewGet());
        if (!ItemDefinition) return;

        bool found = false;
        for (auto& LootDrop : *LootDrops) {
            if (LootDrop->ItemDefinition == ItemDefinition) {
                LootDrop->Count += LootPackage->Count;
                if (LootDrop->Count > ItemDefinition->MaxStackSize.Value) {
                    auto OGCount = LootDrop->Count;
                    LootDrop->Count = ItemDefinition->MaxStackSize.Value;
                    
                    if (Inventory::GetQuickbar(LootDrop->ItemDefinition) == EFortQuickBars::Secondary) LootDrops->Add(Inventory::MakeItemEntry(ItemDefinition, OGCount - ItemDefinition->MaxStackSize.Value, std::clamp(Inventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
                }
                if (Inventory::GetQuickbar(LootDrop->ItemDefinition) == EFortQuickBars::Secondary) found = true;
            }
        }

        if (!found && LootPackage->Count > 0) {
            LootDrops->Add(Inventory::MakeItemEntry(ItemDefinition, LootPackage->Count, std::clamp(Inventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
        }
    }

    bool PickLootDrops(UWorld* WorldContextObject, TArray<FFortItemEntry*>* LootDrops, const FName TierGroupName, const int32 WorldLevel = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel, const int32 ForcedLootTier = -1, bool bForceUpdateTables = false)
    {
        std::vector<FFortLootTierData*> TierDataAllGroups;

        auto LootTierDataTable = Native::StaticLoadObject<UDataTable>(((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist->LootPackages.ObjectID.AssetPathName.ToString().c_str());
        if (!LootTierDataTable) LootTierDataTable = Native::StaticLoadObject<UDataTable>("/Game/Items/Datatables/AthenaLootTierData_Client.AthenaLootTierData_Client");
        if (!LootTierDataTable) return false;

        static UFortPlaylistAthena* LoadedPlaylist = Misc::GetCurrentPlaylist();
        if (!LoadedPlaylist)
            return false;

        LootTierDataTables.push_back(LootTierDataTable);

        if (auto CompositeTable = Util::Cast<UCompositeDataTable>(LootTierDataTable)) {
            for (auto& ParentTable : CompositeTable->ParentTables) {
                for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) ParentTable->RowMap) {
                    if (!Val) continue;
                    TierDataAllGroups.push_back(Val);
                }
            }
        }
        for (UDataTable* LootTierData : LootTierDataTables) {
            if (!LootTierData) continue;
            for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) LootTierData->RowMap) {
                if (!Val) continue;
                TierDataAllGroups.push_back(Val);
            }
        }


        std::vector<FFortLootTierData*> TierDataGroups;
        for (auto& Val : TierDataAllGroups) {
            if (!Val) continue;
            if (Val->TierGroup.ToString() == TierGroupName.ToString() && (ForcedLootTier == -1 ? true : ForcedLootTier == Val->LootTier)) {
                TierDataGroups.push_back(Val);
            }
        }
        auto LootTierData = GetWeightedRow(TierDataGroups);
        if (!LootTierData) return {};

        float DropCount = 0;
        if (LootTierData->NumLootPackageDrops > 0) {
            DropCount = LootTierData->NumLootPackageDrops < 1 ? 1 : (float)((int)((LootTierData->NumLootPackageDrops * 2) - .5f) >> 1);
            if (LootTierData->NumLootPackageDrops > 1) {
                float idk = LootTierData->NumLootPackageDrops - DropCount;
                if (idk > 0.0000099999997f) DropCount += idk >= ((float)rand() / 32767);
            }
        }

        float AmountOfLootDrops = 0;
        float MinLootDrops = 0;

        for (auto& Min : LootTierData->LootPackageCategoryMinArray) AmountOfLootDrops += Min;

        int SumWeights = 0;

        for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i)
        {
            if (LootTierData->LootPackageCategoryWeightArray[i] > 0 && LootTierData->LootPackageCategoryMaxArray[i] != 0) SumWeights += LootTierData->LootPackageCategoryWeightArray[i];
        }

        while (SumWeights > 0)
        {
            AmountOfLootDrops++;

            if (AmountOfLootDrops >= LootTierData->NumLootPackageDrops) {
                AmountOfLootDrops = AmountOfLootDrops;
                break;
            }

            SumWeights--;
        }

        if (!AmountOfLootDrops) AmountOfLootDrops = AmountOfLootDrops;

        for (int i = 0; i < AmountOfLootDrops && i < LootTierData->LootPackageCategoryMinArray.Num(); i++)
        {
            for (int j = 0; j < LootTierData->LootPackageCategoryMinArray[i] && LootTierData->LootPackageCategoryMinArray[i] >= 1; j++)
            {
                SetupLDSForPackage(LootDrops, LootTierData->LootPackage, i, WorldLevel);
            }
        }


        return LootDrops->Num() > 0;
    }
}