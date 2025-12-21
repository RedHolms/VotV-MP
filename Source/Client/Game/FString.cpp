#include "FString.hpp"

#include "FMemory.hpp"

FString::~FString() {
  if (buffer != nullptr)
    FMemory::Free(buffer);
}
