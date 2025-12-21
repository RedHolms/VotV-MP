#pragma once

#include <optional>
#include <stdint.h>
#include <string>
#include <variant>

enum class IPVersion {
  v4, v6
};

struct IPv4 {
  static constexpr IPVersion Version = IPVersion::v4;
  static constexpr std::optional<IPv4> Parse(std::string_view const& addr) noexcept;

  uint32_t addr = 0;

  constexpr std::string toString() const noexcept;

  static constexpr IPv4 Unspecified = Parse("0.0.0.0");
  static constexpr IPv4 LocalHost = Parse("127.0.0.1");
};

struct IPv6 {
  static constexpr IPVersion Version = IPVersion::v6;
  static constexpr std::optional<IPv6> Parse(std::string_view const& addr);

  uint8_t addr[16] = {0};

  constexpr std::string toString() const noexcept;

  static constexpr IPv6 Unspecified = Parse("::");
  static constexpr IPv6 LocalHost = Parse("::1");
};

struct IP : std::variant<IPv4, IPv6> {
  using std::variant<IPv4, IPv6>::variant;

  constexpr IPVersion getVersion() const noexcept {
    return std::visit([](auto const& ip) { return ip.Version; }, *this);
  }

  static constexpr std::optional<IP> Parse(std::string_view const& addr) {
    if (addr.find(':') != std::string::npos)
      return IPv6::Parse(addr);

    return IPv4::Parse(addr);
  }

  constexpr std::string toString() const noexcept {
    return std::visit([](auto const& ip) { return ip.toString(); }, *this);
  }
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

constexpr std::string IPv4::toString() const noexcept {
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

constexpr std::optional<IPv6> IPv6::Parse(std::string_view const& addr) noexcept {
  IPv6 result = {};

  size_t writeIndex = 0;
  ptrdiff_t compressIndex = -1;

  size_t i = 0;
  while (i < addr.size()) {
    if (writeIndex >= 16)
      return std::nullopt;

    if (addr[i] == ':') {
      if (i + 1 < addr.size() && addr[i + 1] == ':') {
        if (compressIndex != -1)
          return std::nullopt;

        compressIndex = (ptrdiff_t)writeIndex;
        ++i;
      }

      ++i;
      continue;
    }

    uint16_t word = 0;
    for (size_t j = 0; i < addr.size() && addr[i] != ':'; ++i, ++j) {
      if (j == 4)
        return std::nullopt;

      word <<= 4;

      uint8_t chr = addr[i];
      if (chr >= '0' && chr <= '9')
        word |= chr - '0';
      else if (chr >= 'a' && chr <= 'f')
        word |= chr - 'a' + 10;
      else if (chr >= 'A' && chr <= 'F')
        word |= chr - 'A' + 10;
      else
        return std::nullopt;
    }

    if (writeIndex + 1 >= 16)
      return std::nullopt;

    result.addr[writeIndex++] = word >> 8;
    result.addr[writeIndex++] = word & 0xFF;
  }

  if (compressIndex != -1) {
    size_t usedBytes = writeIndex;
    size_t tailCount = usedBytes - compressIndex;

    for (size_t j = 0; j < tailCount; ++j)
      result.addr[16 - tailCount + j] = result.addr[compressIndex + j];

    for (size_t j = compressIndex; j < 16 - tailCount; ++j)
      result.addr[j] = 0;
  }
  else if (writeIndex != 16) {
    return std::nullopt;
  }

  return result;
}

constexpr std::string IPv6::toString() const noexcept {
  uint16_t* words = (uint16_t*)addr;

  int bestStart = -1, bestLen = 0;
  for (int i = 0; i < 8;) {
    if (words[i] == 0) {
      int j = i;
      while (j < 8 && words[j] == 0)
        ++j;

      int len = j - i;
      if (len > bestLen) {
        bestStart = i;
        bestLen = len;
      }

      i = j;
    }
    else {
      ++i;
    }
  }

  if (bestLen < 2)
    bestStart = -1;

  std::string result;

  for (int i = 0; i < 8; ++i) {
    if (bestStart == i) {
      result += "::";
      i += bestLen - 1;
      continue;
    }

    if (!result.empty() && result.back() != ':')
      result += ':';

    constexpr char HEX_CHARS[] = "0123456789abcdef";

    if (words[i] == 0) {
      result.push_back('0');
    }
    else {
      uint8_t a = addr[i*2], b = addr[i*2+1];

      char chars[4] = {
        HEX_CHARS[a >> 4], HEX_CHARS[a & 0xF],
        HEX_CHARS[b >> 4], HEX_CHARS[b & 0xF]
      };

      // skip leading zeroes (all 4 can't be zero because of check above)
      size_t j = 0;
      while (chars[j] == '0')
        ++j;

      for (; j < 4; ++j)
        result.push_back(chars[j]);
    }
  }

  return result;
}
