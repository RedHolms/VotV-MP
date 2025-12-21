#pragma once

#include "base.hpp"
#include "FString.hpp"

class UWorld {
public:
  struct Meta;

  struct InitializationValues {
    char __Padding00[8];
  };

public:
  inline FString GetMapName();
};

struct UWorld::Meta {
  static constexpr inline Game::Function<FString(UWorld::*)()> GetMapName {
    "?GetMapName@UWorld@@QEBA?BVFString@@XZ", 0x2F6AF30
  };

  static constexpr inline Game::Function<void(UWorld::*)(InitializationValues)> InitWorld {
    "?InitWorld@UWorld@@QEAAXUInitializationValues@1@@Z", 0x2F6D550
  };
};

FString UWorld::GetMapName() {
  return Meta::GetMapName(this);
}
