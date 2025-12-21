#pragma once

#include "base.hpp"
#include "FLinearColor.hpp"

class AHUD {
public:
  struct Meta;

public:
  inline void DrawLine(float X, float Y, float eX, float eY, FLinearColor color, float thickness);
};

struct AHUD::Meta {
  static constexpr inline Game::Function<void(AHUD::*)()> DrawHUD {
    "?DrawHUD@AHUD@@UEAAXXZ", 0x2B8D350
  };

  static constexpr inline Game::Function<void(AHUD::*)(float X, float Y, float eX, float eY, FLinearColor color, float thickness)> DrawLine {
    "?DrawLine@AHUD@@QEAAXMMMMUFLinearColor@@M@Z", 0x2B8D420
  };
};

void AHUD::DrawLine(float X, float Y, float eX, float eY, FLinearColor color, float thickness) {
  Meta::DrawLine(this, X, Y, eX, eY, color, thickness);
}
