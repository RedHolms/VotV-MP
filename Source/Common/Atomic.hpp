#pragma once

#include <algorithm>
#include <mutex>

template <typename T>
class Locked {
private:
  std::unique_lock<std::mutex> m_lock;
  T& m_value;

public:
  inline Locked(std::mutex& mutex, T& value)
    : m_lock(mutex),
      m_value(value) {}
  inline Locked(std::unique_lock<std::mutex>&& lock, T& value)
    : m_lock(std::move(lock)),
      m_value(value) {}

  inline ~Locked() = default;

  Locked(Locked const&) = delete;
  Locked& operator=(Locked const&) = delete;

public:
  auto* operator->() noexcept {
    return &m_value;
  }
  auto const* operator->() const noexcept {
    return &m_value;
  }
  auto& operator*() noexcept {
    return m_value;
  }
  auto const& operator*() const noexcept {
    return m_value;
  }

  // If locked type is a pointer, lock reference instead
  inline auto Deref() {
    return Locked<decltype(*m_value)>(std::move(m_lock), *m_value);
  }
};

template <typename T>
class Atomic {
private:
  mutable std::mutex m_mutex;
  T m_value;

public:
  template <typename... Args>
  inline explicit Atomic(Args&&... args)
    : m_value(forward<Args>(args)...) {}

  inline ~Atomic() = default;

  Atomic(Atomic const&) = delete;
  Atomic& operator=(Atomic const&) = delete;

public:
  inline Locked<T> Lock() {
    return Locked<T>(m_mutex, m_value);
  }

  inline Locked<T const> Lock() const {
    return Locked<T>(m_mutex, m_value);
  }
};
