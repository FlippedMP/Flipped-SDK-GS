#pragma once
#include "../Other/Framework.h"
#include <numeric>
#include "Inventory.h"



namespace Looting
{
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

    template <typename T>
    static T* PickWeighted(TArray<T*>& Map, float (*RandFunc)(float), bool bCheckZero = true) {
        float TotalWeight = std::accumulate(Map.begin(), Map.end(), 0.0f, [&](float acc, T*& p) { return acc + p->Weight; });
        float RandomNumber = RandFunc(TotalWeight);

        for (auto& Element : Map)
        {
            float Weight = Element->Weight;
            if (bCheckZero && Weight == 0)
                continue;

            if (RandomNumber <= Weight) return Element;

            RandomNumber -= Weight;
        }

        return nullptr;
    }

    inline TArray<FFortLootTierData*> TierDataAllGroups;
    inline TArray<FFortLootPackageData*> LPGroupsAll;

    void SetupLDSForPackage(TArray<FFortItemEntry>& LootDrops, SDK::FName Package, int i, std::string& TierGroup, int WorldLevel = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel) {
        TArray<FFortLootPackageData*> LPGroups;
        for (auto const& Val : LPGroupsAll)
        {
            if (!Val)
                continue;

            if (Val->LootPackageID != Package)
                continue;
            if (i != -1 && Val->LootPackageCategory != i)
                continue;
            if (WorldLevel >= 0) {
                if (Val->MaxWorldLevel >= 0 && WorldLevel > Val->MaxWorldLevel)
                    continue;
                if (Val->MinWorldLevel >= 0 && WorldLevel < Val->MinWorldLevel)
                    continue;
            }

            LPGroups.Add(Val);
        }
        if (LPGroups.Num() == 0)
            return;

        auto LootPackage = PickWeighted(LPGroups, [](float Total) { return ((float)rand() / 32767.f) * Total; });
        if (!LootPackage)
            return;

        if (LootPackage->LootPackageCall.Num() > 1)
        {
            for (int i = 0; i < LootPackage->Count; i++)
                SetupLDSForPackage(LootDrops, UKismetStringLibrary::Conv_StringToName(LootPackage->LootPackageCall), 0, TierGroup);

            return;
        }

        auto ItemDefinition = Util::Cast<UFortWorldItemDefinition>(LootPackage->ItemDefinition.NewGet());
        if (!ItemDefinition)
            return;

        bool found = false;
        for (auto& LootDrop : LootDrops) {
            if (LootDrop.ItemDefinition == ItemDefinition) {
                LootDrop.Count += LootPackage->Count;
                if (LootDrop.Count > ItemDefinition->MaxStackSize.Value) {
                    auto OGCount = LootDrop.Count;
                    LootDrop.Count = ItemDefinition->MaxStackSize.Value;
                    
                    if (Inventory::GetQuickbar(LootDrop.ItemDefinition) == EFortQuickBars::Secondary) LootDrops.Add(*Inventory::MakeItemEntry(ItemDefinition, OGCount - ItemDefinition->MaxStackSize.Value, std::clamp(Inventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
                }
                if (Inventory::GetQuickbar(LootDrop.ItemDefinition) == EFortQuickBars::Secondary) found = true;
            }
        }

        if (!found && LootPackage->Count > 0) {
            LootDrops.Add(*Inventory::MakeItemEntry(ItemDefinition, LootPackage->Count, std::clamp(Inventory::GetLevel(ItemDefinition->LootLevelData), ItemDefinition->MinLevel, ItemDefinition->MaxLevel)));
        }
    }

    void SetupLootGroups(AFortGameStateAthena* GameState, bool bTierDataGroups)
    {
        if (!GameState) return;

        if (bTierDataGroups) {
            LPGroupsAll.Free();
            TierDataAllGroups.Free();
        }
        else {
            if (LPGroupsAll.Num() > 0 && TierDataAllGroups.Num() > 0)
                return;
        }

        static UDataTable* LootPackages = nullptr;
        static UDataTable* LootTierData = nullptr;

        if (!LootPackages || !LootTierData) {
            LootPackages = GameState->CurrentPlaylistInfo.BasePlaylist->LootPackages.NewGet();
            LootTierData = GameState->CurrentPlaylistInfo.BasePlaylist->LootTierData.NewGet();

            if (!LootPackages || !LootTierData)
            {
                LootPackages = Native::StaticLoadObject<UDataTable>("/Game/Items/DataTables/AthenaLootTierData_Client.AthenaLootTierData_Client");
                LootTierData = Native::StaticLoadObject<UDataTable>("/Game/Items/DataTables/AthenaLootPackages_Client.AthenaLootPackages_Client");
            }
        }

        if (LootPackages)
        {
            for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>)LootPackages->RowMap) {
                LPGroupsAll.Add(Val);
            }
        }
        if (LootTierData) {
            for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) LootTierData->RowMap) {
                if (!Val) continue;
                TierDataAllGroups.Add(Val);
            }
        }

        auto GameFeatureDataArray = Misc::GetObjectsOfClass<UFortGameFeatureData>();
        for (const auto& GameFeatureData : GameFeatureDataArray) {
            bool bAppliedPlaylistData = false;
            auto LootTableData = GameFeatureData->DefaultLootTableData;
            for (int i = 0; i < Misc::GetCurrentPlaylist()->GameplayTagContainer.GameplayTags.Num(); i++) {
				auto& Tag = Misc::GetCurrentPlaylist()->GameplayTagContainer.GameplayTags[i];

                for (auto& [CurrentOverideTag, Val] : GameFeatureData->PlaylistOverrideLootTableData)
                {
                    if (Tag == CurrentOverideTag) {
                        UDataTable* PlaylistLootPackages = Val.LootPackageData.NewGet();
						UDataTable* PLaylistLootTierData = Val.LootTierData.NewGet();
                        if (PlaylistLootPackages && PLaylistLootTierData) {
                            for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>)PlaylistLootPackages->RowMap) {
                                LPGroupsAll.Add(Val);
                            }
                            for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) PLaylistLootTierData->RowMap) {
                                if (!Val) continue;
                                TierDataAllGroups.Add(Val);
							}
                        }
                    }
                }
            }
            if (auto LootPackageData = LootTableData.LootPackageData.NewGet())
            {
                for (auto& [Key, Val] : (TMap<FName, FFortLootPackageData*>)LootPackageData->RowMap) {
                    LPGroupsAll.Add(Val);
                }
            }
            if (auto LootTierDataTable = LootTableData.LootTierData.NewGet()) {
                for (auto& [Key, Val] : (TMap<FName, FFortLootTierData*>) LootTierDataTable->RowMap) {
                    if (!Val) continue;
                    TierDataAllGroups.Add(Val);
                }
            }
        }
    }

    TArray<FFortItemEntry> PickLootDrops(std::string& TierGroupName, bool bSetupTables = false, int LootTier = -1, int WorldLevel = ((AFortGameStateAthena*)UWorld::GetWorld()->GameState)->WorldLevel) {
        
        SetupLootGroups(((AFortGameStateAthena*)UWorld::GetWorld()->GameState), bSetupTables);

        if (TierDataAllGroups.Num() == 0 || LPGroupsAll.Num() == 0) {
            std::cout << "No Loot Groups or Tier Data found!" << std::endl;
            return {};
		}

        
        TArray<FFortLootTierData*> TierDataGroups;
        for (auto const& Val : TierDataAllGroups) {
            if (!Val) continue;
            std::string ValTierGroup = Val->TierGroup.IsValid()? Val->TierGroup.ToString() : "";
            if (ValTierGroup == TierGroupName && (LootTier == -1 ? true : LootTier == Val->LootTier))
                TierDataGroups.Add(Val);
        }

        if (TierDataGroups.Num() == 0)
            return {};

        auto LootTierData = PickWeighted(TierDataGroups, [](float Total) { return ((float)rand() / 32767.f) * Total; });
        if (!LootTierData)
            return {};

        float DropCount = 0;
        if (LootTierData->NumLootPackageDrops > 0) {
            DropCount = LootTierData->NumLootPackageDrops < 1 ? 1 : (float)((int)((LootTierData->NumLootPackageDrops * 2) - .5f) >> 1);

            if (LootTierData->NumLootPackageDrops > 1) {
                float idk = LootTierData->NumLootPackageDrops - DropCount;

                if (idk > 0.0000099999997f)
                    DropCount += idk >= ((float)rand() / 32767);
            }
        }

        float AmountOfLootDrops = 0;
        float MinLootDrops = 0;

        for (auto& Min : LootTierData->LootPackageCategoryMinArray)
            AmountOfLootDrops += Min;

        int SumWeights = 0;

        for (int i = 0; i < LootTierData->LootPackageCategoryWeightArray.Num(); ++i)
            if (LootTierData->LootPackageCategoryWeightArray[i] > 0 && LootTierData->LootPackageCategoryMaxArray[i] != 0)
                SumWeights += LootTierData->LootPackageCategoryWeightArray[i];

        while (SumWeights > 0)
        {
            AmountOfLootDrops++;

            if (AmountOfLootDrops >= LootTierData->NumLootPackageDrops) {
                AmountOfLootDrops = AmountOfLootDrops;
                break;
            }

            SumWeights--;
        }

        if (!AmountOfLootDrops)
            AmountOfLootDrops = AmountOfLootDrops;

        TArray<FFortItemEntry> LootDrops;

        for (int i = 0; i < AmountOfLootDrops && i < LootTierData->LootPackageCategoryMinArray.Num(); i++)
            for (int j = 0; j < LootTierData->LootPackageCategoryMinArray[i] && LootTierData->LootPackageCategoryMinArray[i] >= 1; j++)
                SetupLDSForPackage(LootDrops, LootTierData->LootPackage, i, TierGroupName, WorldLevel);

        std::map<UFortWorldItemDefinition*, int32> AmmoMap;
        for (auto& Item : LootDrops)
            if (Item.ItemDefinition->IsA<UFortWeaponRangedItemDefinition>() && !Item.ItemDefinition->IsStackable() && ((UFortWorldItemDefinition*)Item.ItemDefinition)->GetAmmoWorldItemDefinition_BP())
            {
                auto AmmoDefinition = ((UFortWorldItemDefinition*)Item.ItemDefinition)->GetAmmoWorldItemDefinition_BP();
                int i = 0;
                auto AmmoEntry = LootDrops.Search([&](FFortItemEntry& Entry)
                    {
                        if (AmmoMap[AmmoDefinition] > 0 && i < AmmoMap[AmmoDefinition])
                        {
                            i++;
                            return false;
                        }
                        AmmoMap[AmmoDefinition]++;
                        return Entry.ItemDefinition == AmmoDefinition;
                    });

                if (AmmoEntry)
                    continue;

                FFortLootPackageData* Group = nullptr;
                static auto AmmoSmall = UKismetStringLibrary::Conv_StringToName(L"WorldList.AthenaAmmoSmall");
                for (auto const& Val : LPGroupsAll)
                    if (Val->LootPackageID == AmmoSmall && Val->ItemDefinition.NewGet() == AmmoDefinition)
                    {
                        Group = Val;
                        break;
                    }

                if (Group)
                    LootDrops.Add(*Inventory::MakeItemEntry(AmmoDefinition, Group->Count, 0));
            }

        return LootDrops;
    }
}