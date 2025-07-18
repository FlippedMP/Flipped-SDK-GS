#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <thread>
#include <map>
#include <numeric>
#include <chrono>
#include <type_traits>
#include <mutex>
#include "../Includes/SDK/SDK.hpp"
using namespace SDK;
#include "../Includes/MinHook/MinHook.h"
#include "Memory.h"
#include "Logger.h"
#include "../Includes/curl/curl.h"

// ts looks like a magma gs with this many random shit in here! :skull:

#define FLIPPED_LOG(Message) std::cout << "[" << __FUNCTION__ << "]: " << Message << "\n";
#define SET_TITLE(NewTitle) SetConsoleTitleA((std::string(NewTitle) + " | Compiled at " + __TIME__).c_str())

#define DEFINE_INDEX(Idx) (uint32_t)Idx
#define DEFINE_OG(OG) (void**)&OG

static FName NAME_GameNetDriver = UKismetStringLibrary::Conv_StringToName(L"GameNetDriver");

static bool bUsesGameSessions = true;
static constexpr bool bLategame = true;
static constexpr bool bCreative = false;
static constexpr bool bDisableAI = true;
static constexpr bool bLog = false;

enum EHookType
{
	Normal,
	Virtual,
	Patch,
	ModifyInstruction
};

static int ReturnTrue() { return 1; }
static void ReturnHook() { return; }

__forceinline UEngine* GetEngine() { return UEngine::GetEngine(); }
__forceinline UWorld* GetWorld() { return GetEngine()->GameViewport->World; }

namespace Util
{
	template <typename T>
	__forceinline T* Cast(UObject* Object)
	{
		if (Object && Object->Class && Object->IsA(T::StaticClass()))
			return reinterpret_cast<T*>(Object);

		return nullptr;
	}

	class FHookBase
	{
	public:
		static bool Initialize()
		{
			std::cout << "ImageBase: " << std::hex << uintptr_t(GetModuleHandle(0)) << "\n";
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
		uint8_t Byte = 0;
		uint64_t Instruction = 0;
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

		void PatchInternal(uint64_t _Address, uint8_t _Byte)
		{
			DWORD oldProtect;
			VirtualProtect(LPVOID(_Address), sizeof(_Byte), PAGE_EXECUTE_READWRITE, &oldProtect);

			*(uint8_t*)_Address = _Byte;

			VirtualProtect(LPVOID(_Address), sizeof(_Byte), oldProtect, &oldProtect);
		}

		void PatchInternal(uint64_t _Address, uint32_t _Byte)
		{
			DWORD oldProtect;
			VirtualProtect(LPVOID(_Address), sizeof(_Byte), PAGE_EXECUTE_READWRITE, &oldProtect);

			*(uint8_t*)_Address = _Byte;

			VirtualProtect(LPVOID(_Address), sizeof(_Byte), oldProtect, &oldProtect);
		}



		void ModifyInstructionInternal(uintptr_t _Instruction, uintptr_t _NewAddress)
		{
			uint8_t* InstructionAddr = (uint8_t*)_Instruction;
			uint8_t* NewAddr = (uint8_t*)_NewAddress;

			int64_t Relative = (int64_t)(NewAddr - (InstructionAddr + 5));

			int32_t* addr = reinterpret_cast<int32_t*>(InstructionAddr + 1);

			DWORD dwProtection;
			VirtualProtect(addr, sizeof(int32_t), PAGE_EXECUTE_READWRITE, &dwProtection);

			*addr = static_cast<int32_t>(Relative);

			DWORD dwTemp;
			VirtualProtect(addr, sizeof(int32_t), dwProtection, &dwTemp);
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

		FHook(std::string _PatchName, uint64_t _Address, uint8_t _Byte) {
			HookName = _PatchName;
			Type = EHookType::Patch;
			Address = Addresses::ImageBase + _Address;
			Byte = _Byte;

			PatchInternal(Address, Byte);
		}

		FHook(std::string _PatchName, uint64_t _Address, uint32_t _Byte) {
			HookName = _PatchName;
			Type = EHookType::Patch;
			Address = Addresses::ImageBase + _Address;
			Byte = _Byte;

			PatchInternal(Address, Byte);
		}


		FHook(std::string _ModInstructionName, uintptr_t _Instruction, uintptr_t _NewAddress) {
			HookName = _ModInstructionName;
			Type = EHookType::ModifyInstruction;
			Address = Addresses::ImageBase + _NewAddress;
			Instruction = _Instruction;

			ModifyInstructionInternal(_Instruction, _NewAddress);
		}
	};
}

struct FLategameLoadout
{
	UFortItemDefinition* Definition;
	int32 Count;
};

inline std::vector<FLategameLoadout> ARLoadouts;
inline std::vector<FLategameLoadout> ShotgunLoadouts;
inline std::vector<FLategameLoadout> SMGLoadouts;
inline std::vector<FLategameLoadout> FirstConsumableSlotLoadouts;
inline std::vector<FLategameLoadout> SecondConsumableSlotLoadouts;

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

	template <typename T>
	__forceinline std::vector<T*> GetAllActorsOfClass()
	{
		TArray<AActor*> Actors;
		std::vector<AActor*> ActorsVector;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), T::StaticClass(), &Actors);

		if (Actors.Num() == 0)
		{
			FLIPPED_LOG("UGameplayStatics::GetAllActorsOfClass returned 0 actors! Returning empty vector.");
			return ActorsVector;
		}

		ActorsVector.reserve(Actors.Num());
		for (AActor*& CurrentActor : Actors)
		{
			if (!CurrentActor)
				continue;

			ActorsVector.push_back(CurrentActor);
		}

		Actors.Free();
		return ActorsVector;
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

	inline std::vector<std::string> split(std::string s, std::string delimiter) {
		size_t pos_start = 0, pos_end, delim_len = delimiter.length();
		std::string token;
		std::vector<std::string> res;

		while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
			token = s.substr(pos_start, pos_end - pos_start);
			pos_start = pos_end + delim_len;
			res.push_back(token);
		}

		res.push_back(s.substr(pos_start));
		return res;
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

	UAthenaCharacterItemDefinition* GetRandomCharacter() {
		UAthenaCharacterItemDefinition* Ret = nullptr;
		
		static std::vector<UAthenaCharacterItemDefinition*> Characters;
		if (Characters.empty()) {
			Characters = Misc::GetObjectsOfClass<UAthenaCharacterItemDefinition>();
		}

		int RandomIndex = UKismetMathLibrary::RandomIntegerInRange(0, Characters.size());
		Ret = Characters[RandomIndex];
		return Ret;
	}

	UCurveTable* GetGameData()
	{
		auto GameState = Util::Cast<AFortGameStateAthena>(UWorld::GetWorld()->GameState);
		return GameState->AthenaGameDataTable;
	}

	void ApplyDataTablePatch(UDataTable* DataTable)
	{
		std::vector<UFortWeaponRangedItemDefinition*> Items = GetObjectsOfClass<UFortWeaponRangedItemDefinition>();

		for (UFortWeaponRangedItemDefinition* RangedItemDefinition : Items) {
			if (RangedItemDefinition) {
				if (RangedItemDefinition->WeaponStatHandle.DataTable != DataTable) continue;
				for (auto& RowPair : DataTable->RowMap) {
					if (RowPair.First == RangedItemDefinition->WeaponStatHandle.RowName) {
						FFortRangedWeaponStats* WeaponStats = (FFortRangedWeaponStats*)RowPair.Second;
						WeaponStats->KnockbackMagnitude = 0.0;
						WeaponStats->MidRangeKnockbackMagnitude = 0.0;
						WeaponStats->LongRangeKnockbackMagnitude = 0.0;
						WeaponStats->KnockbackZAngle = 0.0;
					}

				}
			}
		}
	}

	UFortPlaylistAthena* GetCurrentPlaylist()
	{
		return Util::Cast<AFortGameStateAthena>(GetWorld()->GameState)->CurrentPlaylistInfo.BasePlaylist;
	}

	void ApplyModifiersToPlayer(AFortPlayerControllerAthena* PC) {
		if (!PC) return;
		auto PlayerState = (AFortPlayerStateAthena*)PC->PlayerState;
		void* InterfaceAddress = Native::GetInterfaceAddress(PlayerState, IAbilitySystemInterface::StaticClass());

		if (!InterfaceAddress)
			return;

		if (!PlayerState->AbilitySystemComponent) return;

		TScriptInterface<IAbilitySystemInterface> Script;
		Script.ObjectPointer = PlayerState;
		Script.InterfacePointer = InterfaceAddress;
		auto Playlist = GetCurrentPlaylist();
		if (!Playlist) return;
		for (const auto& Modifier : Playlist->ModifierList) {
			if (Modifier.IsValid()) {
				UFortGameplayModifierItemDefinition* ModDef = Modifier.NewGet();
				if (ModDef) {
					for (const auto& PersGameplayEffect : ModDef->PersistentGameplayEffects) {
						if (!PersGameplayEffect.DeliveryRequirements.bApplyToPlayerPawns) continue;
						if (PersGameplayEffect.DeliveryRequirements.bConsiderTeam) continue;
						for (const auto& GameplayEffect : PersGameplayEffect.GameplayEffects) {
							FGameplayEffectContextHandle EffectContext{};
							UClass* GameplayEffectClass = GameplayEffect.GameplayEffect.NewGet();
							if (!GameplayEffectClass) continue;
							PlayerState->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(GameplayEffectClass, GameplayEffect.Level, EffectContext);
						}
					}
					for (const auto& PersAbilitySet : ModDef->PersistentAbilitySets) {
						for (const auto& AbilitySet : PersAbilitySet.AbilitySets) {
							UFortKismetLibrary::EquipFortAbilitySet(Script, AbilitySet.NewGet(), nullptr);
						}
					}
				}
			}
		}
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
	void StepExplicitProperty(void* Result, SDK::FProperty* Property) {
		static void (*StepExplicitPropertyOriginal)(__int64, void*, FProperty*) = decltype(StepExplicitPropertyOriginal)(InSDKUtils::GetImageBase() + 0xCC9C90);
		StepExplicitPropertyOriginal(__int64(this), Result, Property);
	}

	void StepCompiledIn(void* const Result)
	{
		if (Code())
		{
			Step(Object(), Result);
		}
		else {
			FProperty* Prop = PropertyChainForCompiledIn();
			PropertyChainForCompiledIn() = (FProperty*)Prop->Next;
			StepExplicitProperty(Result, Prop);
		}
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


template<typename UEType>
UEType* SDK::TSoftClassPtr<UEType>::NewGet() const
{
	std::string String = UKismetStringLibrary::Conv_NameToString(this->ObjectID.AssetPathName).ToString();
	//printf("Getting %s\n", String.c_str());
	if (this->WeakPtr.ObjectIndex != -1)
		return Util::Cast<UEType>(UObject::GObjects->GetByIndex(this->WeakPtr.ObjectIndex));
	else if (this->ObjectID.IsValid())
		return Native::StaticLoadObject<UEType>(String);

	return nullptr;
}

template <typename _Is>
static __forceinline void PatchUse(uintptr_t ptr, _Is byte)
{
	DWORD og;
	VirtualProtect(LPVOID(ptr), sizeof(_Is), PAGE_EXECUTE_READWRITE, &og);
	*(_Is*)ptr = byte;
	VirtualProtect(LPVOID(ptr), sizeof(_Is), og, &og);
}

void VirtualHookInternal(void** _VTable, uint32_t _Index, void* _Detour, void** _OG = nullptr)
{
	if (_VTable)
	{
		DWORD oldProtect;
		VirtualProtect(&_VTable[_Index], sizeof(void*), 0x40, &oldProtect);
		if (_OG)
			*_OG = _VTable[_Index];

		_VTable[_Index] = _Detour;
		VirtualProtect(&_VTable[_Index], sizeof(void*), oldProtect, &oldProtect);
	}
}

void OnBaseUrlResolved(void* Result)
{
	printf(__FUNCTION__);
}

void PostRequest(std::string URL, std::string JSON) {
	CURL* curl;
	CURLcode res;

	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		struct curl_slist* headers = nullptr;
		headers = curl_slist_append(headers, "Content-Type: application/json");
		
		curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, JSON.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, JSON.length());

		res = curl_easy_perform(curl);
		if (res != CURLE_OK)
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;

		curl_slist_free_all(headers);
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
}

SDK::FGameplayTag::FGameplayTag(std::wstring TagName) {
	this->TagName = UKismetStringLibrary::Conv_StringToName(TagName.c_str());
}

inline void ExecHook(UFunction* Function, void* Detour, void** OG = nullptr) {
	if (!Function) {
		printf("Fiailed to get function");
		return;
	}
	if (OG)
		*OG = Function->ExecFunction;

	/*VirtualProtects???*/
	Function->ExecFunction = (UFunction::FNativeFuncPtr)Detour;
}

inline std::map<std::string, int> PlayerToVbucksMap;