#pragma once

#include "Client/Game/FName.hpp"
#include "Client/Game/Game.hpp"
#include "Client/Game/UObjectBaseUtility.hpp"

class UClass;

class UObject : public UObjectBaseUtility {
public:
  forceinline UObject* CreateDefaultSubobject(
    FName subobjectFName,
    UClass* returnType,
    UClass* classToCreateByDefault,
    bool required,
    bool transient
  );
};

static_assert(sizeof(UObject) == 48);

// clang-format off
DEFINE_SYMBOL("?CreateDefaultSubobject@UObject@@QEAAPEAV1@VFName@@PEAVUClass@@1_N2@Z", 0x13F1080, UObject*(UObject::*)(FName subobjectFName, UClass* returnType, UClass* classToCreateByDefault, bool required, bool transient));
// clang-format on

forceinline UObject* UObject::CreateDefaultSubobject(
  FName subobjectFName,
  UClass* returnType,
  UClass* classToCreateByDefault,
  bool required,
  bool transient
) {
  return Game::CallSymbol<{
    "?CreateDefaultSubobject@UObject@@QEAAPEAV1@VFName@@PEAVUClass@@1_N2@Z" }>(
    this, subobjectFName, returnType, classToCreateByDefault, required, transient
  );
}
