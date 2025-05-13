#pragma once

namespace Addresses {
    inline uint64_t GetNetMode = 0xC9EEBC;
    inline uint64_t GIsClient = 0xB30CF9F;
    inline uint64_t GIsServer = 0xB30CFA0;
    inline uint64_t KickPlayer = 0x78857F4;
    inline uint64_t InitListen = 0x515058C;
    inline uint64_t SetWorld = 0x1597AE4;
    inline uint64_t ActorNetMode = 0xCCBE68;//No Need to worry if this is 0x0
    inline uint64_t ChangeGameSessionId = 0x258D0DC;//No Need to worry if this is 0x0
    inline uint64_t CreateNetDriver_Local = 0x159AD28;
    inline uint64_t GetWorldContextFromObject = 0xBA3014;
    inline uint64_t ServerReplicateActors = 0x55497B4;
    inline uint64_t TickFlush = 0xBC72C0;
    inline uint64_t ReadyToStartMatch = 0x5F9CB9C;
    inline uint64_t SpawnDefaultPawnFor = 0x5FA1F18;
    inline uint64_t ServerAcknowledgePossesion = 0x799F6C8;
}
