#pragma once

#include "Client/Game/FFrame.hpp"
#include "Client/Game/Game.hpp"
#include "Client/Game/UObject.hpp"
#include "Client/Game/UStruct.hpp"

class UFunction : public UStruct {
public:
  forceinline void Invoke(UObject* obj, FFrame* stack, void* Z_Param__Result);

private:
  char __Padding00C0[48];
};

static_assert(sizeof(UFunction) == 240);

// clang-format off
DEFINE_SYMBOL("?Invoke@UFunction@@QEAAXPEAVUObject@@AEAUFFrame@@QEAX@Z", 0x12E7680, void(UFunction::*)(UObject* obj, FFrame* stack, void* Z_Param__Result));
// clang-format on

forceinline void UFunction::Invoke(UObject* obj, FFrame* stack, void* Z_Param__Result) {
  return Game::CallSymbol<{ "?Invoke@UFunction@@QEAAXPEAVUObject@@AEAUFFrame@@QEAX@Z" }>(
    this, obj, stack, Z_Param__Result
  );
}
