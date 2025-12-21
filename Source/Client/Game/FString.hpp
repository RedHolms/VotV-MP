#pragma once

#include <stdint.h>

class FString {
public:
  wchar_t* buffer;
  size_t length;
  size_t capacity;

public:
  FString() = delete;
  FString(FString const&) = delete;
  FString& operator=(FString const&) = delete;
  ~FString();
};
