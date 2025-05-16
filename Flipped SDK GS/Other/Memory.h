#pragma once

namespace Addresses {
	inline uint64_t ImageBase = (uint64_t)GetModuleHandle(NULL);

	inline uint64_t GIsClient = Addresses::ImageBase + 0xB30CF9F;
	inline uint64_t GIsServer = GIsClient + 1;

	inline uint64_t ReadyToStartMatch = 0x5F9CB9C;
	inline uint64_t SpawnDefaultPawnFor = 0x5FA1F18;
	inline uint64_t ServerAcknowledgePossession = 0x799F6C8;
	inline uint32_t ServerExecuteInventoryItemVFT = 0x22C;

	inline uint64_t GetMaxTickRate = 0xAED938;
	inline uint64_t TickFlush = 0xBC72C0;
	inline uint64_t WorldGetNetMode = 0xC9EEBC;
	inline uint64_t ActorGetNetMode = 0xCCBE68;
	inline uint64_t GameSessionKickPlayer = 0x78857F4;
	inline uint64_t DispatchRequest = 0x1674270;

	inline uint64_t StaticFindObject = 0x38BFC10;
	inline uint64_t StaticLoadObject = 0x38C1A30;
	inline uint64_t GetWorldContextFromObject = 0xBA3014;
	inline uint64_t CreateNetDriver_Local = 0x159AD28;
	inline uint64_t InitListen = 0x515058C;
	inline uint64_t SetWorld = 0x1597AE4;
	inline uint64_t ServerReplicateActors = 0x55497B4;

	inline std::vector<uint64_t> NullFunctions =
	{
		0x258D0DC // ChangeGamesessionId
	};
}

namespace Native {
	inline UObject* (*StaticFindObject_)(UClass*, UObject*, const wchar_t*, bool)
		= decltype(StaticFindObject_)(Addresses::ImageBase + Addresses::StaticFindObject);

	inline UObject* (*StaticLoadObject_)(UClass*, UObject*, const TCHAR*, const TCHAR*, uint32_t, UObject*, bool)
		= decltype(StaticLoadObject_)(Addresses::ImageBase + Addresses::StaticLoadObject);
	
	template <typename T>
	inline T* FindObject(std::string ObjectPath)
	{
		UClass* _UObjectClass = StaticClassImpl<"Object">();

		T* _ = (T*)StaticFindObject_(_UObjectClass, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), false);
		if (!_)
			return (T*)StaticLoadObject_(_UObjectClass, nullptr, std::wstring(ObjectPath.begin(), ObjectPath.end()).c_str(), nullptr, 0, nullptr, false);

		return nullptr;
	}

	inline void* (*GetWorldFromContextObject)(UEngine*, UWorld*) =
		decltype(GetWorldFromContextObject)(Addresses::ImageBase + Addresses::GetWorldContextFromObject);
	
	inline UNetDriver* (*CreateNetDriver_Local)(UEngine*, void* WorldContext, FName)
		= decltype(CreateNetDriver_Local)(Addresses::ImageBase + Addresses::CreateNetDriver_Local);
	
	inline bool (*InitListen)(UNetDriver*, UWorld*, FURL&, bool, FString)
		= decltype(InitListen)(Addresses::ImageBase + Addresses::InitListen);
	
	inline void (*ServerReplicateActors)(UReplicationDriver*)
		= decltype(ServerReplicateActors)(Addresses::ImageBase + Addresses::ServerReplicateActors);
	
	inline void (*SetWorld)(UNetDriver*, UWorld*)
		= decltype(SetWorld)(Addresses::ImageBase + Addresses::SetWorld);
}