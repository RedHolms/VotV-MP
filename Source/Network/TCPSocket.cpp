#include "TCPSocket.hpp"

TCPSocket::TCPSocket(IPVersion ipVersion)
  : RawSocket(socket(IPVersionToAF(ipVersion), SOCK_STREAM, IPPROTO_TCP)) {}

TCPSocket::~TCPSocket() = default;

void TCPSocket::Bind(SocketAddress const& address) {
  auto systemAddress = address.ToSystem();
  bind(m_handle, &systemAddress.addr, systemAddress.addrLen);
}

void TCPSocket::Listen(int backlog) {
  listen(m_handle, backlog);
}

TCPSocket TCPSocket::Accept() {}

void TCPSocket::Connect(SocketAddress const& address) {
  auto systemAddress = address.ToSystem();
  connect(m_handle, &systemAddress.addr, systemAddress.addrLen);
}

bool TCPSocket::IsConnected() {}
