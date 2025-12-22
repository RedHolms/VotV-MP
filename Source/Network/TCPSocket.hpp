#pragma once

#include "Common/Bytes.hpp"
#include "Network/IPVersion.hpp"
#include "Network/RawSocket.hpp"
#include "Network/SocketAddress.hpp"

class TCPSocket : public RawSocket {
public:
  explicit TCPSocket(IPVersion ipVersion);
  ~TCPSocket();

public:
  void Bind(SocketAddress const& address);
  void Listen(int backlog);
  TCPSocket Accept();

  void Connect(SocketAddress const& address);

  bool IsConnected();
};
