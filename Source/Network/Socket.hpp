#pragma once

#include <Windows.h>
#include <WinSock2.h>
#include "SocketAddress.hpp"

enum class Protocol {
  TCP,
  UDP
};

class Socket {
private:
  SOCKET m_handle;

public:
  Socket(IPVersion ipVersion, Protocol protocol) {
    int af, tp, proto;
    switch (ipVersion) {
      default:
      case IPVersion::v4:
        af = AF_INET;
        break;
      case IPVersion::v6:
        af = AF_INET6;
        break;
    }

    switch (protocol) {
      default:
      case Protocol::TCP:
        tp = SOCK_STREAM;
        proto = IPPROTO_TCP;
        break;
      case Protocol::UDP:
        tp = SOCK_DGRAM;
        proto = IPPROTO_UDP;
        break;
    }

    m_handle = socket(af, tp, proto);
  }

  ~Socket() {
    closesocket(m_handle);
  }

  Socket(Socket const&) = delete;
  Socket(Socket&&) = delete;
  Socket& operator=(Socket const&) = delete;
  Socket& operator=(Socket&&) = delete;

public:
  void sendTo(const void* buffer, size_t bytesCount, SocketAddress const& destination) {
    auto sockaddr = destination.toSystem();
    sendto(m_handle, (const char*)buffer, bytesCount, 0, &sockaddr.addr, sockaddr.addrLen);
  }

  size_t receiveFrom(void* buffer, size_t bufferCapacity, SocketAddress* outSource) {

  }
};
