#pragma once
#include "../Other/Framework.h"

namespace AI
{
	void SpawnPhoebeAI(int Count) {
		if (bLategame)
			return;
		auto GameMode = Util::Cast<AFortGameModeAthena>(UWorld::GetWorld()->AuthorityGameMode);
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), AFortPlayerStartWarmup::StaticClass(), &PlayerStarts);
		for (int i = 0; i < Count; i++) {
			static UClass* Class = Native::StaticLoadObject<UClass>("/Game/Athena/AI/Phoebe/BP_AISpawnerData_Phoebe.BP_AISpawnerData_Phoebe_C");
			static UFortAthenaAISpawnerDataComponentList* List = UFortAthenaAISpawnerData::CreateComponentListFromClass(Class, UWorld::GetWorld());
			static UBehaviorTree* BehaviorTree = Native::StaticLoadObject<UBehaviorTree>("/Game/Athena/AI/Phoebe/BehaviorTrees/BT_Phoebe.BT_Phoebe");

			int Num = UKismetMathLibrary::RandomIntegerInRange(1, PlayerStarts.Num());
			printf("Num: %d\n", Num);
			if (!PlayerStarts.IsValidIndex(Num))
				Num = UKismetMathLibrary::RandomIntegerInRange(1, PlayerStarts.Num());
			AActor* PlayerStart = PlayerStarts[Num];

			auto AISystem = (UAthenaAISystem*)UWorld::GetWorld()->AISystem;
			auto AISpawner = AISystem->AISpawner;
			AISpawner->RequestSpawn(List, PlayerStart->GetTransform());
		}
	}

	void SpawnKlombo(FTransform Transform, int Count) {
		if (bLategame)
			return;
		static UClass* Class = Native::StaticLoadObject<UClass>("/ButterCake/AISpawnerData/Base/AISpawnerData_ButterCake_A.AISpawnerData_ButterCake_A_C");
		if (!Class) {
			printf("No Class");
			return;
		}
		static UFortAthenaAISpawnerDataComponentList* List = UFortAthenaAISpawnerData::CreateComponentListFromClass(Class, UWorld::GetWorld());
		if (!List) {
			printf("No List");
			return;
		}
		static UBehaviorTree* BehaviorTree = Native::StaticLoadObject<UBehaviorTree>("/ButterCake/BehaviorTree/BT_ButterCake.BT_ButterCake");
		if (!BehaviorTree) {
			printf("no Behaviortree");
			return;
		}

		auto AISystem = (UAthenaAISystem*)UWorld::GetWorld()->AISystem;
		auto AISpawner = AISystem->AISpawner;
		AISpawner->RequestSpawn(List, Transform);
	}


	void GetDances(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* thisPtr, TArray<UAthenaDanceItemDefinition*>* Dances, const AFortAthenaAIBotController* Controller) {
		//printf(__FUNCTION__"\n");
		static std::vector<UAthenaDanceItemDefinition*> DanceArray;

		if (DanceArray.empty()) {
			DanceArray = Misc::GetObjectsOfClass<UAthenaDanceItemDefinition>();
		}

		for (UAthenaDanceItemDefinition* Dance : DanceArray) {
			Dances->Add(Dance);
		}

		//printf("BT: %s\n", Controller->BehaviorTree->GetFullName().c_str());
	}

	std::vector<UAthenaCharacterItemDefinition*> ChosenCharacters;

	void GetLoadout(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* thisPtr, FFortAthenaLoadout* OutLoadout) {
		OutLoadout->Character = Misc::GetRandomCharacter();
		ChosenCharacters.push_back(OutLoadout->Character);
	}

	void GetInventoryItems(UFortAthenaAISpawnerDataComponent_InventoryBase* thisPtr, TArray<FItemAndCount>* OutList) {
		auto GameMode = (AFortGameModeAthena*)UWorld::GetWorld()->AuthorityGameMode;
		for (auto& StartingItem : GameMode->StartingItems) {
			if (StartingItem.Item->GetFullName().contains("Smart")) {
				OutList->Add(StartingItem);
			}
		}

		FItemAndCount ItemAndCount{};
		ItemAndCount.Item = UObject::FindObject<UAthenaPickaxeItemDefinition>("AthenaPickaxeItemDefinition DefaultPickaxe.DefaultPickaxe")->WeaponDefinition;
		ItemAndCount.Count = 1;
		OutList->Add(ItemAndCount);
	}

	void UpdateInventory(AFortAthenaAIBotController* Controller, FFortItemEntry* ItemEntry = nullptr)
	{
		Controller->Inventory->bRequiresLocalUpdate = true;
		Controller->Inventory->HandleInventoryLocalUpdate();

		if (ItemEntry)
			Controller->Inventory->Inventory.MarkItemDirty(*ItemEntry);
		else
			Controller->Inventory->Inventory.MarkArrayDirty();
	}

	UFortWorldItem* AddItem(AFortAthenaAIBotController* Controller, UFortItemDefinition* Definition, int Count = 1) {
		if (!Controller || !Controller->Inventory || !Definition)
			return nullptr;

		UE_LOG(LogFlipped, Log, "Adding %s to %s's Inventory", Definition->GetFullName().c_str(), Controller->GetFullName().c_str());

		UFortWorldItem* Item = Util::Cast<UFortWorldItem>(Definition->CreateTemporaryItemInstanceBP(Count, 0));
		Item->OwnerInventory = Controller->Inventory;

		Item->ItemEntry.ItemDefinition = Definition;
		Item->ItemEntry.Count = Count;
		Item->ItemEntry.Level = 0;

		Controller->Inventory->Inventory.ItemInstances.Add(Item);
		Controller->Inventory->Inventory.ReplicatedEntries.Add(Item->ItemEntry);

		UpdateInventory(Controller, &Item->ItemEntry);

		if (auto Item2 = Util::Cast<UFortWeaponMeleeItemDefinition>(Definition)) {
			Controller->EquipWeapon(Item);
		}

		return Item;
	}

	void (*PostOnSpawnedOG)(UFortAthenaAISpawnerDataComponent_InventoryBase*, AFortAIPawn*);
	void PostOnSpawned(UFortAthenaAISpawnerDataComponent_InventoryBase* thisPtr, AFortAIPawn* PawnAI) {
		AFortAthenaAIBotController* Controller = Util::Cast<AFortAthenaAIBotController>(PawnAI->Controller);
		Controller->Inventory = Misc::SpawnActor<AFortInventory>({},{}, Controller);

		TArray<FItemAndCount> StartingItems;
		GetInventoryItems(thisPtr, &StartingItems);

		for (const auto& StartingItem : StartingItems) {
			AddItem(Controller, StartingItem.Item, StartingItem.Count);
		}

		

		PostOnSpawnedOG(thisPtr, PawnAI);
	}
}