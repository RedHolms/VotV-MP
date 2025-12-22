#pragma once

#include "Network/IPv4.hpp"
#include "Network/IPv6.hpp"
#include <variant>

struct IP : std::variant<IPv4, IPv6> {
  using std::variant<IPv4, IPv6>::variant;

  constexpr IPVersion GetVersion() const noexcept {
    return std::visit([](auto const& ip) { return ip.Version; }, *this);
  }

  static constexpr std::optional<IP> Parse(std::string_view const& addr) {
    if (addr.find(':') != std::string::npos)
      return IPv6::Parse(addr);

    return IPv4::Parse(addr);
  }

  constexpr std::string ToString() const noexcept {
    return std::visit([](auto const& ip) { return ip.ToString(); }, *this);
  }
};
