#pragma once

#include "Client/Game/FString.hpp"
#include "Client/Game/Game.hpp"

class UWorld {
public:
  struct InitializationValues {
    char __Padding00[8];
  };

public:
  forceinline FString GetMapName();
  forceinline void InitWorld(InitializationValues initValues);
};

// clang-format off
DEFINE_SYMBOL("?GetMapName@UWorld@@QEBA?BVFString@@XZ", 0x2F6AF30, FString(UWorld::*)());
DEFINE_SYMBOL("?InitWorld@UWorld@@QEAAXUInitializationValues@1@@Z", 0x2F6D550, void(UWorld::*)(UWorld::InitializationValues));
// clang-format on

forceinline FString UWorld::GetMapName() {
  return Game::CallSymbol<{ "?GetMapName@UWorld@@QEBA?BVFString@@XZ" }>(this);
}

forceinline void UWorld::InitWorld(InitializationValues initValues) {
  Game::CallSymbol<{ "?InitWorld@UWorld@@QEAAXUInitializationValues@1@@Z" }>(this, initValues);
}
