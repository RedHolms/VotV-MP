#pragma once

#include <stdint.h>

class Bytes {
public:
  constexpr Bytes() = default;

  constexpr Bytes(void const* data, size_t bytesCount) noexcept
    : m_data(data),
      m_bytesCount(bytesCount) {}

public:
  void const* GetData() const noexcept {
    return m_data;
  }

  size_t GetBytesCount() const noexcept {
    return m_bytesCount;
  }

private:
  void const* m_data = nullptr;
  size_t m_bytesCount = 0;
};
