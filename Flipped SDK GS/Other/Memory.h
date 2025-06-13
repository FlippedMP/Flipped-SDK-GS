#pragma once

namespace Addresses 
{
	const inline uint64_t ImageBase = (uint64_t)GetModuleHandle(NULL);

	const inline uint64_t GIsClient = Addresses::ImageBase + 0xB30CF9F;
	const inline uint64_t GIsServer = GIsClient + 1;

	const inline uint64_t ReadyToStartMatch = 0x5F9CB9C;
	const inline uint64_t SpawnDefaultPawnFor = 0x5FA1F18;
	const inline uint64_t StartNewSafeZonePhase = 0x5FA67BC;
	const inline uint32_t GetGameSessionClassVFT = 0xE3;
	const inline uint64_t ServerAcknowledgePossession = 0x799F6C8;
	const inline uint32_t ServerExecuteInventoryItemVFT = 0x22C;
	const inline uint32_t ServerTryActivateAbilityWithEventDataVFT = 0x108;
	const inline uint32_t ServerLoadingScreenDroppedVFT = 0x28E;

	const inline uint64_t SpawnLoot = 0x617CCBC;
	const inline uint32_t ServerAttemptAircraftJumpVFT = 0x9E;
	const inline uint64_t OnAircraftEnteredDropZone = 0x5F99274;
	const inline uint64_t OnAircraftExitedDropZone = 0x5F992F8;

	const inline uint32_t ServerCreateBuildingActorVFT = 0x24D;
	const inline uint32_t ServerRepairBuildingActorVFT = 0x249;
	const inline uint32_t ServerBeginEditingBuildingActorVFT = 0x254;
	const inline uint32_t ServerEditBuildingActorVFT = 0x24F;
	const inline uint32_t ServerEndEditingBuildingActorVFT = 0x252;

	const inline uint64_t GetMaxTickRate = 0xAED938;
	const inline uint64_t TickFlush = 0xBC72C0;
	const inline uint64_t WorldGetNetMode = 0xC9EEBC;
	const inline uint64_t ActorGetNetMode = 0xCCBE68;
	const inline uint64_t GameSessionKickPlayer = 0x78857F4;
	const inline uint64_t DispatchRequest = 0x1674270;

	const inline uint64_t StaticFindObject = 0xbc9b40;
	const inline uint64_t StaticLoadObject = 0x10a6fb8;
	const inline uint64_t GetWorldContextFromObject = 0xBA3014;
	const inline uint64_t CreateNetDriver_Local = 0x159AD28;
	const inline uint64_t InitListen = 0x515058C;
	const inline uint64_t SetWorld = 0x1597AE4;
	const inline uint64_t ServerReplicateActors = 0x55497B4;
	const inline uint64_t GetInterfaceAddress = 0xB1B28C;
	const inline uint64_t InternalTryActivateAbility = 0x4E02108;

	const inline uint64_t OnGamePhaseStepChanged = 0x6A69F80; // AFortAthenaMutator_GiveItemsAtGamePhaseStep
	
	const inline uint64_t SpawnAI = 0x69D5510;
	const inline uint64_t CreateAndConfigureNavigationSystem = 0x1F96F68;

	const inline uint64_t CanCreateInCurrentContext = 0xD708E0;

	const inline uint64_t GameSessionPatch = 0x65B510F + 1;
	const inline uint64_t FCommandLineGetCommandLine = 0xB71D9C;
	const inline uint64_t ServicePermissionsByName = 0x17F5A4C;

	const inline std::vector<uint64_t> NullFunctions =
	{
		0x258D0DC // ChangeGamesessionId
	};
}

template <typename StorageType, typename FuncType>
struct TFunctionRefBase;

template <typename StorageType, typename Ret, typename... ParamTypes>
struct TFunctionRefBase<StorageType, Ret(ParamTypes...)>
{
	Ret(*Callable)(void*, ParamTypes&...);

	StorageType Storage;
};

struct FFunctionStorage
{
	FFunctionStorage()
		: HeapAllocation(nullptr)
	{
	}

	void* HeapAllocation;
};




namespace Native 
{
	inline UObject* (*StaticFindObject_)(UClass*, UObject*, const wchar_t*, bool)
		= decltype(StaticFindObject_)(Addresses::ImageBase + Addresses::StaticFindObject);

	inline UObject* (*StaticLoadObject_)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool)
		= decltype(StaticLoadObject_)(Addresses::ImageBase + Addresses::StaticLoadObject);
	
	template <typename T>
	inline T* StaticFindObject(std::string ObjectPath, UClass* Class = UObject::StaticClass())
	{
		return (T*)StaticFindObject_(Class, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false);
	}

	template <typename T>
	inline T* StaticLoadObject(std::string Path, UClass* InClass = T::StaticClass(), UObject* Outer = nullptr)
	{
		return (T*)StaticLoadObject_(InClass, Outer, std::wstring(Path.begin(), Path.end()).c_str(), nullptr, 0, nullptr, false);
	}

	inline void* (*GetWorldFromContextObject)(UEngine*, UWorld*) =
		decltype(GetWorldFromContextObject)(Addresses::ImageBase + Addresses::GetWorldContextFromObject);
	
	inline UNetDriver* (*CreateNetDriver_Local)(UEngine*, void*, FName)
		= decltype(CreateNetDriver_Local)(Addresses::ImageBase + Addresses::CreateNetDriver_Local);
	
	inline bool (*InitListen)(UNetDriver*, UWorld*, FURL&, bool, FString)
		= decltype(InitListen)(Addresses::ImageBase + Addresses::InitListen);
	
	inline void (*ServerReplicateActors)(UReplicationDriver*)
		= decltype(ServerReplicateActors)(Addresses::ImageBase + Addresses::ServerReplicateActors);
	
	inline void (*SetWorld)(UNetDriver*, UWorld*)
		= decltype(SetWorld)(Addresses::ImageBase + Addresses::SetWorld);

	inline void* (*GetInterfaceAddress)(UObject*, UClass*) 
		= decltype(GetInterfaceAddress)(Addresses::ImageBase + Addresses::GetInterfaceAddress);

	inline bool (*InternalTryActivateAbility)(UAbilitySystemComponent*, FGameplayAbilitySpecHandle, FPredictionKey, UGameplayAbility**, void*, const FGameplayEventData*)
		= decltype(InternalTryActivateAbility)(Addresses::ImageBase + Addresses::InternalTryActivateAbility);

	inline EFortStructuralGridQueryResults (*CanPlaceBuildableClassInStructuralGrid)(UObject*, UObject*, FVector Location, FRotator Rotation, bool bMirrored, TArray<ABuildingActor*>* ExistingBuildings, char* MarkerOptionalAdjustment)
		= decltype(CanPlaceBuildableClassInStructuralGrid)(Addresses::ImageBase + 0x63FCF40);

	inline ABuildingSMActor* (*ReplaceBuildingActor)(ABuildingSMActor*, unsigned int, UObject*, int, int, char, AFortPlayerControllerAthena*)
		= decltype(ReplaceBuildingActor)(Addresses::ImageBase + 0x61B1AB4);
}