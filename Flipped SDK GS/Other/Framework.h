#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <thread>
#include "../Includes/SDK/SDK.hpp"
using namespace SDK;
#include "../Includes/MinHook/MinHook.h"
#include "Memory.h"

// ts looks like a magma gs with this many random shit in here! :skull:

#define FLIPPED_LOG(Message) std::cout << "[" << __FUNCTION__ << "]: " << Message << "\n";
#define SET_TITLE(NewTitle) SetConsoleTitleA((std::string(NewTitle) + " | Compiled at " + __TIME__).c_str())

#define DEFINE_INDEX(Idx) (uint32_t)Idx
#define DEFINE_OG(OG) (void**)&OG

static FName NAME_GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

static bool bUsesGameSessions = false;

enum EHookType
{
	Normal,
	Virtual,
	Patch
};

static constexpr int bLategame = 1;

static int ReturnTrue() { return 1; }
static void ReturnHook() { return; }

__forceinline UEngine* GetEngine() { return UEngine::GetEngine(); }
__forceinline UWorld* GetWorld() { return GetEngine()->GameViewport->World; }

namespace Util
{
	template <typename T>
	__forceinline T* Cast(UObject* Object)
	{
		if (Object && Object->IsA(T::StaticClass()))
			return reinterpret_cast<T*>(Object);

		return nullptr;
	}

	class FHookBase
	{
	public:
		static bool Initialize()
		{
			return (MH_Initialize() == MH_OK);
		}
	};

	template <typename UEClass = UObject> // if it works we dont change it head ass code right here!
	struct FHook : public FHookBase
	{
	private:
		void* Detour = nullptr;
		void** VTable = nullptr;
		uint32_t Index = 0;
		uint64_t Address = 0;
		EHookType Type = Normal;
		void** OG = nullptr;
		std::string HookName;

		void VirtualHookInternal(void** _VTable, uint32_t _Index, void* _Detour, void** _OG = nullptr)
		{
			if (_VTable)
			{
				DWORD oldProtect;
				VirtualProtect(&_VTable[_Index], sizeof(void*), 0x40, &oldProtect);
				if (_OG)
					*_OG = _VTable[_Index];

				_VTable[_Index] = Detour;
				VirtualProtect(&_VTable[_Index], sizeof(void*), oldProtect, &oldProtect);
			}
		}

		void HookInternal(uint64_t _Address, void* _Detour, void** _OG = nullptr)
		{
			MH_CreateHook((LPVOID)_Address, _Detour, _OG);
			MH_EnableHook((LPVOID)_Address);
		}

		static void Patch(uintptr_t ptr, uint8_t byte)
		{
			DWORD og;
			VirtualProtect(LPVOID(ptr), sizeof(byte), PAGE_EXECUTE_READWRITE, &og);
			*(uint8_t*)ptr = byte;
			VirtualProtect(LPVOID(ptr), sizeof(byte), og, &og);
		}

	public:
		FHook(std::string _HookName, uint32_t _Index, void* _Detour, void** _OG = nullptr)
		{
			HookName = _HookName;
			VTable = UEClass::GetDefaultObj()->VTable;
			Index = _Index;
			Detour = _Detour;
			OG = _OG;
			Type = Virtual;

			if (UEClass::StaticClass() == UObject::StaticClass())
			{
				for (int i = 0; i < UObject::GObjects->Num(); i++)
				{
					UObject* Object = UObject::GObjects->GetByIndex(i);
					if (Object)
					{
						VirtualHookInternal(Object->VTable, Index, Detour, OG);
					}
				}
				return;
			}

			VirtualHookInternal(VTable, Index, Detour, OG);
		}

		FHook(std::string _HookName, uint64_t _Address, void* _Detour, void** _OG = nullptr)
		{
			HookName = _HookName;
			Address = Addresses::ImageBase + _Address;
			Detour = _Detour;
			OG = _OG;
			Type = Normal;

			HookInternal(Address, Detour, OG);
		}

		FHook(std::string _PatchName, uint64_t _Address, uint8_t Byte) {
			HookName = _PatchName;
			Type = EHookType::Patch;
			Address = Addresses::ImageBase + _Address;

			Patch(Address, Byte);
		}
	};
}

namespace Misc
{
	template <typename T>
	__forceinline T* SpawnActor(const FVector& Location = {}, const FRotator& Rotation = {}, AActor* Owner = nullptr, UClass* Class = T::StaticClass())
	{
		if (!Class)
		{
			FLIPPED_LOG("Invalid class when trying to spawn actor!");
			return nullptr;
		}

		FTransform NewTransform;
		NewTransform.Translation = Location;
		NewTransform.Rotation = UKismetMathLibrary::Conv_RotatorToQuaternion(Rotation); // I LOVE UE5!!!!
		NewTransform.Scale3D = FVector(1, 1, 1);

		AActor* NewActor = UGameplayStatics::BeginDeferredActorSpawnFromClass(GetWorld(), Class, NewTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, Owner);
		if (!NewActor)
			return nullptr;

		AActor* FinishingNewActor = UGameplayStatics::FinishSpawningActor(NewActor, NewTransform);
		if (!FinishingNewActor)
			return nullptr;

		return Util::Cast<T>(FinishingNewActor);
	}

	float RandRange(float min, float max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dist(min, max);

		return dist(gen);
	}

	FVector GetRandomLocationInCircle(const FVector& Center, float Radius) {
		float angle = RandRange(0.0f, 2.0f * 3.14159265f);
		float distance = sqrtf(RandRange(0.0f, 1.0f)) * Radius;
		float x = cosf(angle) * distance;
		float y = sinf(angle) * distance;

		return Center + FVector(x, y, 0.0f);
	}

	template<typename T>
	std::vector<T*> GetObjectsOfClass(UClass* Class = T::StaticClass())
	{
		std::vector<T*> ArrayOfObjects;
		for (int i = 0; i < UObject::GObjects->Num(); i++)
		{
			UObject* Object = UObject::GObjects->GetByIndex(i);

			if (!Object)
				continue;

			if (Object->IsA(Class))
			{
				ArrayOfObjects.push_back(Util::Cast<T>(Object));
			}
		}

		return ArrayOfObjects;
	}

	UCurveTable* GetGameData() {
		return Native::StaticFindObject<UCurveTable>("/Game/Athena/Balance/DataTables/AthenaGameData.AthenaGameData");
	}

	void ApplyDataTablePatch(UDataTable* DataTable) {
		if (!DataTable) return;
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(UWorld::GetWorld(), UFortWeaponRangedItemDefinition::StaticClass(), &Actors);
		for (AActor* Actor : Actors) {
			UFortWeaponRangedItemDefinition* RangedItemDefinition = reinterpret_cast<UFortWeaponRangedItemDefinition*>(Actor);
			if (RangedItemDefinition) {
				FFortRangedWeaponStats WeaponStats;
				UDataTableFunctionLibrary::GetDataTableRowFromName(DataTable, RangedItemDefinition->GetWeaponStatHandle().RowName, &WeaponStats);
				WeaponStats.KnockbackMagnitude = 0.0;
				WeaponStats.MidRangeKnockbackMagnitude = 0.0;
				WeaponStats.LongRangeKnockbackMagnitude = 0.0;
				WeaponStats.KnockbackZAngle = 0.0;
			}
		}

		Actors.Free();
	}

	UFortPlaylistAthena* CurrentPlaylist()
	{
		return Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist;
	}

	static void ModifyInstruction_Internal(uintptr_t Instruction, uintptr_t NewAddress)
	{
		uint8_t* InstructionAddr = (uint8_t*)Instruction;
		uint8_t* NewAddr = (uint8_t*)NewAddress;

		int64_t Relative = (int64_t)(NewAddr - (InstructionAddr + 5));

		int32_t* addr = reinterpret_cast<int32_t*>(InstructionAddr + 1);

		DWORD dwProtection;
		VirtualProtect(addr, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &dwProtection);

		*addr = static_cast<int32_t>(Relative);

		DWORD dwTemp;
		VirtualProtect(addr, sizeof(int32_t), dwProtection, &dwTemp);
	}

	template <typename _Is>
	static __forceinline void Patch(uintptr_t ptr, _Is byte)
	{
		DWORD og;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
		*(_Is*)ptr = byte;
		VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);
	}
}

class FFrame
{
public:
	unsigned __int8* MostRecentPropertyAddress() {
		return *reinterpret_cast<unsigned __int8**>(uint64_t(this) + 0x38);
	}

	unsigned __int8*& Code() {
		return *reinterpret_cast<unsigned __int8**>(uint64_t(this) + 0x20);
	}
	SDK::FProperty*& PropertyChainForCompiledIn() {
		return *reinterpret_cast<FProperty**>(uint64_t(this) + 0x80);
	}
	SDK::UObject* Object() {
		return *reinterpret_cast<UObject**>(uint64_t(this) + 0x18);
	}


public:


	void Step(SDK::UObject* Context, void* const RESULT_PARAM) {
		static void (*StepOriginal)(__int64, SDK::UObject*, void* const RESULT_PARAM) = decltype(StepOriginal)(InSDKUtils::GetImageBase() + 0xCCB6B8);
		StepOriginal(__int64(this), Context, RESULT_PARAM);
	}
	void StepExplicitProperty(unsigned __int8* Result, SDK::FProperty* Property) {
		static void (*StepExplicitPropertyOriginal)(__int64, unsigned __int8*, FProperty*) = decltype(StepExplicitPropertyOriginal)(InSDKUtils::GetImageBase() + 0xCC9C90);
		StepExplicitPropertyOriginal(__int64(this), Result, Property);
	}
};

class FServicePermissionsMcp
{
public:
	FString Name;
	FString Id;
	FString Key;
};


template<typename UEType>
UEType* SDK::TSoftObjectPtr<UEType>::NewGet() const
{
	std::string String = UKismetStringLibrary::Conv_NameToString(this->ObjectID.AssetPathName).ToString();
	//printf("Getting %s\n", String.c_str());
	if (this->WeakPtr.ObjectIndex != -1)
		return Util::Cast<UEType>(UObject::GObjects->GetByIndex(this->WeakPtr.ObjectIndex));
	else if (this->ObjectID.IsValid())
		return Native::StaticLoadObject<UEType>(String);

	return nullptr;
}