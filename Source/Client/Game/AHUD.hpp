#pragma once

#include "Client/Game/FLinearColor.hpp"
#include "Client/Game/Game.hpp"

class AHUD {
public:
  forceinline void DrawHUD();

  forceinline void DrawLine(
    float X,
    float Y,
    float eX,
    float eY,
    FLinearColor color,
    float thickness
  );
};

// clang-format off
DEFINE_SYMBOL("?DrawHUD@AHUD@@UEAAXXZ", 0x2B8D350, void(AHUD::*)());
DEFINE_SYMBOL("?DrawLine@AHUD@@QEAAXMMMMUFLinearColor@@M@Z", 0x2B8D420, void(AHUD::*)(float X, float Y, float eX, float eY, FLinearColor color, float thickness));
// clang-format on

forceinline void AHUD::DrawHUD() {
  Game::CallSymbol<{ "?DrawHUD@AHUD@@UEAAXXZ" }>(this);
}

forceinline void AHUD::DrawLine(
  float X,
  float Y,
  float eX,
  float eY,
  FLinearColor color,
  float thickness
) {
  Game::CallSymbol<{ "?DrawLine@AHUD@@QEAAXMMMMUFLinearColor@@M@Z" }>(
    this, X, Y, eX, eY, color, thickness
  );
}
