#pragma once

#include "Client/Game/FString.hpp"
#include "Client/Game/UObject.hpp"

class UField : public UObject {
public:
  forceinline FString GetAuthoredName() const;

private:
  UField* Next;
};

static_assert(sizeof(UField) == 56);

// clang-format off
DEFINE_SYMBOL("?GetAuthoredName@FField@@QEBA?AVFString@@XZ", 0x1303E10, FString(UField::*)() const);
// clang-format on

forceinline FString UField::GetAuthoredName() const {
  return Game::CallSymbol<{ "?GetAuthoredName@FField@@QEBA?AVFString@@XZ" }>(this);
}
