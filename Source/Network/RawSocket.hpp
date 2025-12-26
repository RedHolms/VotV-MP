#pragma once

#if _OS_WINDOWS
#include <WinSock2.h>
#elif _OS_LINUX
#error TODO
#endif

// Smart-socket object. Shouldn't be used directly
class RawSocket {
  NONCOPYABLE_CLASS(RawSocket);

public:
  constexpr RawSocket() : m_handle(INVALID_SOCKET) {}
  constexpr explicit RawSocket(SOCKET handle) : m_handle(handle) {}

  inline ~RawSocket() {
    Invalidate();
  }

  constexpr RawSocket(RawSocket&& other) noexcept : m_handle(other.m_handle) {
    other.m_handle = INVALID_SOCKET;
  }

  RawSocket& operator=(RawSocket&& other) noexcept {
    Invalidate();
    m_handle = other.m_handle;
    other.m_handle = INVALID_SOCKET;
    return *this;
  }

public:
  constexpr SOCKET GetSystemHandle() const noexcept {
    return m_handle;
  }

  constexpr bool IsValid() const noexcept {
    return m_handle != INVALID_SOCKET;
  }

  void Invalidate();

protected:
  SOCKET m_handle;
};
