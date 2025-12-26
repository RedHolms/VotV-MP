#pragma once

#include "Game.hpp"

class IEngineLoop;

class UEngine {
public:
  forceinline void Init(IEngineLoop* engineLoop);
};

// clang-format off
DEFINE_SYMBOL("?Init@UEngine@@UEAAXPEAVIEngineLoop@@@Z", 0x2F183F0, void(UEngine::*)(IEngineLoop*));
// clang-format on

forceinline void UEngine::Init(IEngineLoop* engineLoop) {
  Game::CallSymbol<{ "?Init@UEngine@@UEAAXPEAVIEngineLoop@@@Z" }>(this, engineLoop);
}
