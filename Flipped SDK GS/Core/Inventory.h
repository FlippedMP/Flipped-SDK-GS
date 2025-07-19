#pragma once
#include "../Other/Framework.h"

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

namespace Inventory
{
	void UpdateInventory(AFortPlayerControllerAthena* Controller, FFortItemEntry* ItemEntry = nullptr)
	{
		Controller->WorldInventory->bRequiresLocalUpdate = true;
		Controller->WorldInventory->HandleInventoryLocalUpdate();

		if (ItemEntry)
			Controller->WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
		else
			Controller->WorldInventory->Inventory.MarkArrayDirty();
	}



	int GetLevel(const FDataTableCategoryHandle& CategoryHandle)
	{
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		auto GameState = (AFortGameStateAthena*)GameMode->GameState;

		if (!CategoryHandle.DataTable)
			return 0;

		if (!CategoryHandle.RowContents.ComparisonIndex)
			return 0;

		TArray<FFortLootLevelData*> LootLevelData;

		for (auto& LootLevelDataPair : CategoryHandle.DataTable->RowMap)
		{
			FFortLootLevelData* Value = reinterpret_cast<FFortLootLevelData*>(LootLevelDataPair.Value());
			if (Value->Category != CategoryHandle.RowContents)
				continue;

			LootLevelData.Add(Value);
		}

		if (LootLevelData.Num() > 0)
		{
			int ind = -1;
			int ll = 0;

			for (int i = 0; i < LootLevelData.Num(); i++)
			{
				if (LootLevelData[i]->LootLevel <= GameState->WorldLevel && LootLevelData[i]->LootLevel > ll)
				{
					ll = LootLevelData[i]->LootLevel;
					ind = i;
				}
			}

			if (ind != -1)
			{
				auto subbed = LootLevelData[ind]->MaxItemLevel - LootLevelData[ind]->MinItemLevel;

				if (subbed <= -1)
					subbed = 0;
				else
				{
					auto calc = (int)(((float)rand() / 32767) * (float)(subbed + 1));
					if (calc <= subbed)
						subbed = calc;
				}

				return subbed + LootLevelData[ind]->MinItemLevel;
			}
		}

		return 0;
	}

	int GetClipSize(UFortItemDefinition* Definition)
	{
		if (!Definition)
			return 0;
		if (auto WeaponDef = Util::Cast<UFortWeaponRangedItemDefinition>(Definition)) {
			for (auto& RowPair : WeaponDef->WeaponStatHandle.DataTable->RowMap) {
				if (RowPair.First == WeaponDef->WeaponStatHandle.RowName) {
					return ((FFortRangedWeaponStats*)RowPair.Second)->ClipSize;
				}
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
				LoadedAmmo = Inventory::GetClipSize(ItemDefinition);

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

	int GetMaxStackSize(UFortItemDefinition* Definition)
	{
		if (!Definition)
			return 0;

		float Val = Definition->MaxStackSize.Value;

		if (Val <= 1) {
			FSimpleCurve* Curve = nullptr;
			static UCurveTable* AthenaGameData = Misc::GetGameData();
			for (auto& [Key, Value] : AthenaGameData->GetRowMap()) {
				if (Key == Definition->MaxStackSize.Curve.RowName) {
					Curve = (FSimpleCurve*)Value;
					break;
				}
			}

			if (Curve)
				Val = Curve->Keys[0].Value;
		}
		return Val;
	}

	EFortQuickBars GetQuickbar(UFortItemDefinition* ItemDefinition)
	{
		if (!ItemDefinition) return EFortQuickBars::Max_None;
		return ItemDefinition->IsA<UFortWeaponMeleeItemDefinition>() || ItemDefinition->IsA<UFortResourceItemDefinition>() || ItemDefinition->IsA<UFortAmmoItemDefinition>() || ItemDefinition->IsA<UFortTrapItemDefinition>() || ItemDefinition->IsA<UFortBuildingItemDefinition>() || ItemDefinition->IsA<UFortEditToolItemDefinition>() || ((UFortWorldItemDefinition*)ItemDefinition)->bForceIntoOverflow ? EFortQuickBars::Secondary : EFortQuickBars::Primary;
	}

	void BP_FindExistingItemsForItemDefinition(AFortInventory* Inventory, TArray<UFortWorldItem*>* OutItems, UFortItemDefinition* ItemDefinition)
	{
		for (auto& ItemInstance : Inventory->Inventory.ItemInstances) {
			if (ItemInstance->ItemEntry.ItemDefinition == ItemDefinition)
				OutItems->Add(ItemInstance);
		}
	}

	/* this needs WAY more checks but my brain power isnt enough anymore to do good inv funcs, maybe ill paste for an old gs */
	UFortWorldItem* AddItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Definition, int Count = 1) {
		if (!Controller || !Controller->WorldInventory || !Definition)
			return nullptr;

		if (UFortWorldItem* Item = Util::Cast<UFortWorldItem>(Definition->CreateTemporaryItemInstanceBP(Count, 0))) {
			Item->SetOwningControllerForTemporaryItem(Controller);
			Item->OwnerInventory = Controller->WorldInventory;


			Item->ItemEntry.ItemDefinition = Definition;
			Item->ItemEntry.Count = Count;
			Item->ItemEntry.Level = 0;
			Item->ItemEntry.LoadedAmmo = GetClipSize(Definition);

			if (Definition->IsStackable()) {
				int MaxStackSize = GetMaxStackSize(Definition);
				for (int i = 0; i < Controller->WorldInventory->Inventory.ItemInstances.Num(); i++) {
					UFortWorldItem* IndexedItem = Controller->WorldInventory->Inventory.ItemInstances[i];
					if (IndexedItem->ItemEntry.ItemDefinition == Definition) {
						Item->ItemEntry.Count += IndexedItem->ItemEntry.Count;
						if (Item->ItemEntry.Count > MaxStackSize) {
							int AmountToRemove = Item->ItemEntry.Count - round(MaxStackSize);
							Item->ItemEntry.Count -= AmountToRemove;
							if (Controller->MyFortPawn) {
								FSpawnPickupData Data{};
								Data.ItemDefinition = Definition;
								Data.Location = Controller->MyFortPawn->K2_GetActorLocation();
								Data.Count = AmountToRemove;
								Data.FortPickupSourceTypeFlag = EFortPickupSourceTypeFlag::Player;
								Data.FortPickupSpawnSource = EFortPickupSpawnSource::Unset;
								Data.PickupOwner = Controller->MyFortPawn;
								SpawnPickup(Data);
							}
						}

						Controller->WorldInventory->Inventory.ItemInstances.Remove(i);
						break;
					}
				}
				for (int f = 0; f < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); f++) {
					FFortItemEntry& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[f];

					if (Entry.ItemDefinition == Definition) {
						Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(f);
					}
				}
			}

			Controller->WorldInventory->Inventory.ItemInstances.Add(Item);
			Controller->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
			UpdateInventory(Controller, &Item->ItemEntry);

			auto WorldItemDef = Util::Cast<UFortWorldItemDefinition>(Definition);
			if (WorldItemDef && WorldItemDef->bForceFocusWhenAdded) {
				Controller->ServerExecuteInventoryItem(Item->ItemEntry.ItemGuid);
				Controller->ClientEquipItem(Item->ItemEntry.ItemGuid, true);
			}

			return Item;
		}
		else
		{
			FLIPPED_LOG("Failed to create item with definition: " + Definition->GetFullName());
			return nullptr;
		}
	}


	UFortWorldItem* AddItem(AFortPlayerControllerAthena* Controller, FFortItemEntry* ItemEntryPtr) {
		return AddItem(Controller, ItemEntryPtr->ItemDefinition, ItemEntryPtr->Count);
	}

	void RemoveItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Definition, int Count = -1) {
		if (!Controller || !Definition)
			return;

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			FFortItemEntry& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition == Definition) {
				if (Count == -1) {
					Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					Controller->WorldInventory->Inventory.MarkArrayDirty();
					for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
						auto& Instance = Controller->WorldInventory->Inventory.ItemInstances[j];
						if (!Instance && j == Controller->WorldInventory->Inventory.ItemInstances.Num() - 1)
							return;

						if (Instance && Instance->ItemEntry.ItemDefinition == Definition) {
							Controller->WorldInventory->Inventory.ItemInstances.Remove(j);
							Controller->WorldInventory->Inventory.MarkArrayDirty();
							break;
						}
					}

					return;
				}
				else {
					if (Entry.Count - Count <= 0) {
						Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
						for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
							auto& Instance = Controller->WorldInventory->Inventory.ItemInstances[j];
							if (!Instance && j == Controller->WorldInventory->Inventory.ItemInstances.Num() - 1)
								return;

							if (Instance && Instance->ItemEntry.ItemDefinition == Definition) {
								Controller->WorldInventory->Inventory.ItemInstances.Remove(j);
								Controller->WorldInventory->Inventory.MarkArrayDirty();
								break;
							}
						}

						return;
					}
					else {
						Entry.PreviousCount = Entry.Count;
						Entry.Count -= Count;
						UpdateInventory(Controller, &Entry);
					}
				}
			}
		}
	}

	void RemoveItem(AFortPlayerControllerAthena* Controller, FGuid ItemGUID, int Count = -1) {
		if (!Controller)
			return;

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			auto& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemGuid == ItemGUID) {
				if (Count == -1) {
					Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					Controller->WorldInventory->Inventory.MarkArrayDirty();
					for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
						auto& Instance = Controller->WorldInventory->Inventory.ItemInstances[j];
						if (!Instance && j == Controller->WorldInventory->Inventory.ItemInstances.Num() - 1)
							return;

						if (Instance->ItemEntry.ItemGuid == ItemGUID) {
							Controller->WorldInventory->Inventory.ItemInstances.Remove(j);
							Controller->WorldInventory->Inventory.MarkArrayDirty();
							break;
						}
					}

					return;
				}
				else {
					if (Entry.Count - Count <= 0) {



						Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
						for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
							auto& Instance = Controller->WorldInventory->Inventory.ItemInstances[j];
							if (!Instance && j == Controller->WorldInventory->Inventory.ItemInstances.Num() - 1)
								return;

							if (Instance && Instance->ItemEntry.ItemGuid == ItemGUID) {
								Controller->WorldInventory->Inventory.ItemInstances.Remove(j);
								Controller->WorldInventory->Inventory.MarkArrayDirty();
								break;
							}
						}

						return;
					}
					else {
						Entry.PreviousCount = Entry.Count;
						Entry.Count -= Count;
						UpdateInventory(Controller, &Entry);
					}
				}
			}
		}
	}

	void RemoveAllDroppableItems(AFortPlayerControllerAthena* Controller) {
		if (!Controller)
			return;

		FLIPPED_LOG(__FUNCTION__);

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ItemInstances.Num(); i++) {
			if (Controller->WorldInventory->Inventory.ItemInstances[i]->CanBeDropped()) {
				Controller->WorldInventory->Inventory.ItemInstances.Remove(i);
				Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
				Controller->WorldInventory->Inventory.MarkArrayDirty();
			}
		}

		UpdateInventory(Controller);
	}

#define MAX_DURABILITY 0x3F800000

	FFortItemEntry* MakeItemEntry(UFortItemDefinition* ItemDefinition, int32 Count, int32 Level) {
		FFortItemEntry* IE = new FFortItemEntry();

		IE->MostRecentArrayReplicationKey = -1;
		IE->ReplicationID = -1;
		IE->ReplicationKey = -1;

		IE->ItemDefinition = ItemDefinition;
		IE->Count = Count;
		IE->LoadedAmmo = GetClipSize(ItemDefinition);
		IE->Durability = MAX_DURABILITY;
		IE->GameplayAbilitySpecHandle = FGameplayAbilitySpecHandle(-1);
		IE->ParentInventory.ObjectIndex = -1;
		IE->Level = Level;

		return IE;
	}

	UFortWorldItem* GetItemFromGUID(AFortPlayerControllerAthena* Controller, FGuid GUID)
	{
		TArray<UFortWorldItem*>& Items = Controller->WorldInventory->Inventory.ItemInstances;
		for (UFortWorldItem*& Item : Items)
			if (Item->ItemEntry.ItemGuid == GUID)
				return Item;

		return nullptr;
	}

	int32 GetNumQuickBarItems(AFortPlayerControllerAthena* Controller) {
		int32 Num = 0;
		for (UFortWorldItem* ItemInstance : Controller->WorldInventory->Inventory.ItemInstances) {
			if (GetQuickbar(ItemInstance->ItemEntry.ItemDefinition) == EFortQuickBars::Primary)
				Num++;
		}

		return Num;
	}

	std::vector<FFortItemEntry*> GetMatchingItems(AFortPlayerControllerAthena* Object, UFortItemDefinition* Def) {
		std::vector<FFortItemEntry*> Entries;
		for (auto& Instance : Object->WorldInventory->Inventory.ItemInstances) {
			if (Instance->ItemEntry.ItemDefinition == Def) {
				Entries.push_back(&Instance->ItemEntry);
			}
		}

		return Entries;
	}

	std::vector<UFortItemDefinition*> GetAllItems(AFortPlayerControllerAthena* PC)
	{
		std::vector<UFortItemDefinition*> DiddyBlud;
		for (const auto& Entry : PC->WorldInventory->Inventory.ReplicatedEntries) {
			DiddyBlud.push_back(Entry.ItemDefinition);
		}
		return DiddyBlud;
	}
}