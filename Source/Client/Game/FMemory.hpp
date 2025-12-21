#pragma once

#include "base.hpp"

struct FMemory {
  struct Meta;

  static inline void Free(void* buffer);
};

struct FMemory::Meta {
  static constexpr inline Game::Function<void(*)(void* buffer)> Free {
    "?Free@FMemory@@SAXPEAX@Z", 0x115CCC0
  };
};

void FMemory::Free(void* buffer) {
  Meta::Free(buffer);
}
