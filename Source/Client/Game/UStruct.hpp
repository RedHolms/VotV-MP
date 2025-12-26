#pragma once

#include "Client/Game/UField.hpp"

class UStruct : public UField {
private:
  char __Padding0038[132];
};

static_assert(sizeof(UStruct) == 192);
