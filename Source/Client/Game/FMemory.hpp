#pragma once

#include "Game.hpp"

struct FMemory {
  forceinline static void Free(void* buffer);
};

// clang-format off
DEFINE_SYMBOL("?Free@FMemory@@SAXPEAX@Z", 0x115CCC0, void(*)(void* buffer));
// clang-format on

forceinline void FMemory::Free(void* buffer) {
  Game::CallSymbol<{ "?Free@FMemory@@SAXPEAX@Z" }>(buffer);
}
