#pragma once

#include "base.hpp"

class IEngineLoop;

class UEngine {
public:
  struct Meta;
};

struct UEngine::Meta {
  static constexpr inline Game::Function<void(UEngine::*)(IEngineLoop*)> Init {
    "?Init@UEngine@@UEAAXPEAVIEngineLoop@@@Z", 0x2F183F0
  };
};
