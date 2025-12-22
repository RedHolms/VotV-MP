#include "SocketAddress.hpp"

std::optional<SocketAddress> SocketAddress::Parse(std::string_view const& addr) {
  size_t portBegin = -1;
  uint16_t port = 0;
  IP ip;

  if (addr[0] == '[') { // IPv6
    size_t ipEnd = addr.find_last_of(']') + 1;

    if (ipEnd != addr.size()) {
      if (addr[ipEnd] != ':')
        return std::nullopt;

      portBegin = ipEnd + 1;
    }

    auto opt = IPv6::Parse(addr.substr(0, ipEnd));
    if (!opt.has_value())
      return std::nullopt;

    ip = opt.value();
  }
  else { // IPv4
    portBegin = addr.find_first_of(':') + 1;

    auto opt = IPv4::Parse(addr.substr(0, portBegin - 1));
    if (!opt.has_value())
      return std::nullopt;

    ip = opt.value();
  }

  if (portBegin != -1) {
    auto portStr = addr.substr(portBegin);

    try {
      port = std::stoul(std::string(portStr));
      if (port > 0xFFFF)
        return std::nullopt;
    }
    catch (...) {
      return std::nullopt;
    }
  }

  return SocketAddress { ip, port };
}

SystemSocketAddress SocketAddress::ToSystem() const noexcept {
  switch (ip.GetVersion()) {
    case IPVersion::v4: {
      auto [addr] = std::get<IPv4>(ip);

      sockaddr_in addr4 = {};
      addr4.sin_family = AF_INET;
      addr4.sin_addr.s_addr = htonl(addr);
      addr4.sin_port = htons(port);

      return SystemSocketAddress(addr4);
    }
    case IPVersion::v6: {
      auto [addr] = std::get<IPv6>(ip);

      sockaddr_in6 addr6 = {};
      addr6.sin6_family = AF_INET6;
      memcpy(addr6.sin6_addr.s6_addr, addr, sizeof(addr));
      addr6.sin6_port = htons(port);

      return SystemSocketAddress(addr6);
    }
    default:
      UNREACHABLE();
  }
}
