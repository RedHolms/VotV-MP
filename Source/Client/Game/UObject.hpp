#pragma once

#include "base.hpp"
#include "FName.hpp"

class UClass;

class UObject {
public:
  struct Meta;

public:
  inline UObject* CreateDefaultSubobject(FName subobjectFName, UClass* returnType, UClass* classToCreateByDefault, bool required, bool transient);
};

struct UObject::Meta {
  static constexpr inline Game::Function<UObject*(UObject::*)(FName subobjectFName, UClass* returnType, UClass* classToCreateByDefault, bool required, bool transient)> CreateDefaultSubobject {
    "?CreateDefaultSubobject@UObject@@QEAAPEAV1@VFName@@PEAVUClass@@1_N2@Z", 0x13F1080
  };
};

UObject* UObject::CreateDefaultSubobject(FName subobjectFName, UClass* returnType, UClass* classToCreateByDefault, bool required, bool transient) {
  return Meta::CreateDefaultSubobject(this, subobjectFName, returnType, classToCreateByDefault, required, transient);
}
