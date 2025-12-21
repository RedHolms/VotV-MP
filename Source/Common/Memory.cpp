#include "Memory.hpp"

Memory::UnFuckScope::UnFuckScope(AnyPtr address, size_t size) noexcept {
  m_address = address.get<void>();
  m_size = size;
  VirtualProtect(m_address, size, PAGE_EXECUTE_READWRITE, (DWORD*)&m_prev);
}

Memory::UnFuckScope::~UnFuckScope() noexcept {
  VirtualProtect(m_address, m_size, m_prev, (DWORD*)&m_prev);
}

bool Memory::TryRead(ConstAnyPtr address, AnyPtr destination, size_t bytesCount) {
  __try {
    memcpy(destination, address, bytesCount);
    return true;
  }
  __except(1) {
    return false;
  }
}