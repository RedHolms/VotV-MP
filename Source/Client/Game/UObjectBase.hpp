#pragma once

class UObjectBase {
private:
  char __Padding0000[40];

private:
  virtual void __VFTableFiller() = 0;
};

static_assert(sizeof(UObjectBase) == 48);
