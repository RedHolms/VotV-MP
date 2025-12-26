#pragma once

#include "Network/IPVersion.hpp"
#include <optional>
#include <stdint.h>
#include <string>

struct IPv4 {
  static constexpr IPVersion Version = IPVersion::v4;

  static constexpr std::optional<IPv4> Parse(std::string_view const& addr) noexcept;

  uint32_t addr = 0;

  constexpr std::string ToString() const noexcept;
};

constexpr std::optional<IPv4> IPv4::Parse(std::string_view const& addr) noexcept {
  uint32_t result = 0;
  uint8_t part = 0;

  size_t partIndex = 0;

  for (auto chr : addr) {
    if (chr == '.') {
      if (partIndex == 3)
        return std::nullopt;

      result <<= 8;
      result |= part;
      part = 0;
      ++partIndex;

      continue;
    }

    if (chr < '0' || chr > '9')
      return std::nullopt;

    if (part != 0)
      part *= 10;

    part += chr - '0';
  }

  if (partIndex != 3)
    return std::nullopt;

  return IPv4 { result };
}

constexpr std::string IPv4::ToString() const noexcept {
  std::string result;
  result += std::to_string((addr >> 24) & 0xFF);
  result += '.';
  result += std::to_string((addr >> 16) & 0xFF);
  result += '.';
  result += std::to_string((addr >> 8) & 0xFF);
  result += '.';
  result += std::to_string(addr & 0xFF);
  return result;
}
