#pragma once
#include "../Other/Framework.h"
#include "./Looting.h"

namespace Inventory
{
	void UpdateInventory(AFortPlayerControllerAthena* Controller, FFortItemEntry* ItemEntry = nullptr)
	{
		if (ItemEntry)
			Controller->WorldInventory->Inventory.MarkItemDirty(*ItemEntry);
		else
			Controller->WorldInventory->Inventory.MarkArrayDirty();

		Controller->HandleWorldInventoryLocalUpdate();
		Controller->WorldInventory->bRequiresLocalUpdate = true;
		Controller->WorldInventory->HandleInventoryLocalUpdate();
	}

	void UpdateEntry(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Definition, int Count, int LoadedAmmo = -1) {
		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			auto& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition == Definition) {
				Entry.PreviousCount = Entry.Count;
				Entry.Count += Count;

				if (LoadedAmmo != -1)
					Entry.LoadedAmmo = LoadedAmmo;

				UpdateInventory(Controller, &Entry);
				return;
			}
		}
	}

	void AddItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Definition, int Count = 1, int LoadedAmmo = 0) {
		if (!Controller || !Definition)
			return;

		bool bItemExists = false;
		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			FFortItemEntry& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition == Definition) {
				bItemExists = true;
				break;
			}
		}

		if (!bItemExists) {
			if (UFortWorldItem* Item = Util::Cast<UFortWorldItem>(Definition->CreateTemporaryItemInstanceBP(Count, 0))) {
				Item->SetOwningControllerForTemporaryItem(Controller);
				Item->OwnerInventory = Controller->WorldInventory;

				Item->ItemEntry.ItemDefinition = Definition;
				Item->ItemEntry.PreviousCount = 0; // well, technically true??
				Item->ItemEntry.Count = Count;
				Item->ItemEntry.LoadedAmmo = LoadedAmmo;

				Controller->WorldInventory->Inventory.ItemInstances.Add(Item);
				Controller->WorldInventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);
				UpdateInventory(Controller, &Item->ItemEntry);

				return;
			}
			else {
				std::cout << "Failed to create item with definition: " << Definition->GetFullName() << std::endl;
				return;
			}
		}
		else {
			if (Definition->IsStackable())
				UpdateEntry(Controller, Definition, Count, LoadedAmmo);
		}
	}

	void RemoveItem(AFortPlayerControllerAthena* Controller, UFortItemDefinition* Definition, int Count = -1) {
		if (!Controller || !Definition)
			return;

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ReplicatedEntries.Num(); i++) {
			auto& Entry = Controller->WorldInventory->Inventory.ReplicatedEntries[i];
			if (Entry.ItemDefinition == Definition) {
				if (Count == -1) {
					Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
					Controller->WorldInventory->Inventory.MarkArrayDirty();
					for (size_t j = 0; j < Controller->WorldInventory->Inventory.ItemInstances.Num(); j++) {
						auto& Instance = Controller->WorldInventory->Inventory.ItemInstances[j];
						if (!Instance && j == Controller->WorldInventory->Inventory.ItemInstances.Num() - 1)
							return;

						if (Instance->ItemEntry.ItemDefinition == Definition) {
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

							if (Instance->ItemEntry.ItemDefinition == Definition) {
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

		for (size_t i = 0; i < Controller->WorldInventory->Inventory.ItemInstances.Num(); i++) {
			if (Controller->WorldInventory->Inventory.ItemInstances[i]->CanBeDropped()) {
				Controller->WorldInventory->Inventory.ItemInstances.Remove(i);
				Controller->WorldInventory->Inventory.ReplicatedEntries.Remove(i);
				Controller->WorldInventory->Inventory.MarkArrayDirty();
			}
		}
	}

	UFortWorldItem* GetItemFromGUID(AFortPlayerControllerAthena* Controller, FGuid GUID)
	{
		TArray<UFortWorldItem*>& Items = Controller->WorldInventory->Inventory.ItemInstances;
		for (UFortWorldItem*& Item : Items)
			if (Item->ItemEntry.ItemGuid == GUID)
				return Item;

		return nullptr;
	}
}