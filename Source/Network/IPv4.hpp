#pragma once

#include <optional>
#include <stdint.h>
#include <string>

struct IPv4 {
  static constexpr uint32_t Version = 4;

  uint32_t addr = 0;

  static constexpr std::optional<IPv4> Parse(std::string_view const& addr) noexcept;

  static constexpr IPv4 Unspecified = Parse("0.0.0.0");
  static constexpr IPv4 LocalHost = Parse("127.0.0.1");

  constexpr std::string toString() const noexcept;
};
