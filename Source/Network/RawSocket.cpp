#include "RawSocket.hpp"

#include "Logging/Asserts.hpp"

void RawSocket::Invalidate() {
  closesocket(m_handle);
  m_handle = INVALID_SOCKET;
}
