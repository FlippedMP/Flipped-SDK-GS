#pragma once

inline uint64_t ImageBase = uint64_t(GetModuleHandle(0)); 
namespace Funcs {
    inline void* (*GetWorldFromContextObject)(SDK::UEngine*, SDK::UWorld*) = decltype(GetWorldFromContextObject)(ImageBase + Addresses::GetWorldContextFromObject);
    inline UNetDriver* (*CreateNetDriver_Local)(SDK::UEngine*, void* WorldContext, FName) = decltype(CreateNetDriver_Local)(ImageBase + Addresses::CreateNetDriver_Local);
    inline bool (*InitListen)(SDK::UNetDriver*, SDK::UWorld*, SDK::FURL&, bool, FString) = decltype(InitListen)(ImageBase + Addresses::InitListen);
    inline void (*ServerReplicateActors)(SDK::UReplicationDriver*, float) = decltype(ServerReplicateActors)(ImageBase + Addresses::ServerReplicateActors);
    inline void (*SetWorld)(SDK::UNetDriver*, SDK::UWorld*) = decltype(SetWorld)(ImageBase + Addresses::SetWorld);
}
