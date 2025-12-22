#pragma once

#include "IP.hpp"
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <stdexcept>

struct SystemSocketAddress {
  union {
    sockaddr addr;
    sockaddr_in addr4;
    sockaddr_in6 addr6;
  };
  socklen_t addrLen;

  constexpr explicit SystemSocketAddress(sockaddr_in const& addr4)
    : addr4(addr4),
      addrLen(sizeof(sockaddr_in)) {}
  constexpr explicit SystemSocketAddress(sockaddr_in6 const& addr6)
    : addr6(addr6),
      addrLen(sizeof(sockaddr_in6)) {}
};

struct SocketAddress {
  IP ip;
  uint16_t port;

  constexpr SocketAddress() : port(0) {}
  constexpr SocketAddress(IP const& ip, uint16_t port) : ip(ip), port(port) {}
  constexpr SocketAddress(IP&& ip, uint16_t port) : ip(std::move(ip)), port(port) {}

  // Port will be set to 0 if not specified
  static std::optional<SocketAddress> Parse(std::string_view const& addr);

  SystemSocketAddress ToSystem() const noexcept;
};
