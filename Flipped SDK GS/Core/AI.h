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
		printf(__FUNCTION__"\n");
		static std::vector<UAthenaDanceItemDefinition*> DanceArray;

		if (DanceArray.empty()) {
			DanceArray = Misc::GetObjectsOfClass<UAthenaDanceItemDefinition>();
		}

		for (UAthenaDanceItemDefinition* Dance : DanceArray) {
			Dances->Add(Dance);
		}
	}

	void GetLoadout(UFortAthenaAISpawnerDataComponent_AIBotCosmeticBase* thisPtr, FFortAthenaLoadout* OutLoadout) {
		printf(__FUNCTION__"\n");
		OutLoadout->Character = Misc::GetRandomCharacter();
		printf("Character: %s\n", OutLoadout->Character);
	}
}