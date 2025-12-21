#pragma once

#include <stdint.h>
#include <optional>

// Pointer to immutable memory without specific type
struct ConstAnyPtr {
  uintptr_t value = 0;

  constexpr ConstAnyPtr() noexcept = default;

  inline implicit ConstAnyPtr(void const* ptr) noexcept
    : value(reinterpret_cast<uintptr_t>(ptr)) {}

  template <std::integral T>
  inline implicit ConstAnyPtr(T value) noexcept
    : value(static_cast<uintptr_t>(value)) {}

  template <typename T>
  [[nodiscard]] inline T const* get() const noexcept {
    return reinterpret_cast<T*>(value);
  }

  inline operator uintptr_t() const noexcept {
    return value;
  }

  inline operator ptrdiff_t() const noexcept {
    return static_cast<ptrdiff_t>(value);
  }

  inline operator void const*() const noexcept {
    return reinterpret_cast<void const*>(value);
  }

  inline ConstAnyPtr& operator++() noexcept {
    ++value;
    return *this;
  }

  inline ConstAnyPtr& operator--() noexcept {
    --value;
    return *this;
  }

  [[nodiscard]] inline ptrdiff_t operator-(ptrdiff_t other) const noexcept {
    return static_cast<ptrdiff_t>(value) - other;
  }

  [[nodiscard]] inline uintptr_t operator+(ptrdiff_t other) const noexcept {
    return value + other;
  }
};

// Pointer to mutable memory without specific type
struct AnyPtr : ConstAnyPtr {
  using ConstAnyPtr::ConstAnyPtr;

  inline implicit AnyPtr(void* ptr) noexcept
    : ConstAnyPtr(reinterpret_cast<uintptr_t>(ptr)) {}

  template <typename T>
  [[nodiscard]] inline T* get() const noexcept {
    return reinterpret_cast<T*>(value);
  }

  inline operator void*() const noexcept {
    return reinterpret_cast<void*>(value);
  }
};

namespace Memory {

// Removes memory protection from a region while in scope
struct UnFuckScope {
public:
  UnFuckScope(AnyPtr address, size_t size) noexcept;
  ~UnFuckScope() noexcept;

private:
  uint32_t m_prev;
  void* m_address;
  size_t m_size;
};

inline void Copy(AnyPtr destination, ConstAnyPtr source, size_t size) noexcept {
  UnFuckScope _scope(destination, size);

  uint8_t* dst = destination.get<uint8_t>();
  uint8_t const* src = source.get<uint8_t>();

  for (size_t i = 0; i < size; ++i)
    dst[i] = src[i];
}

template <typename T>
inline void Fill(AnyPtr destination, size_t count, T value) {
  UnFuckScope _scope(destination, count * sizeof(T));

  T* dst = destination.get<T>();

  for (size_t i = 0; i < count; ++i)
    dst[i] = value;
}

template <typename T>
inline void Write(AnyPtr address, T value) noexcept {
  UnFuckScope _scope(address, sizeof(T));

  address.get<T>()[0] = value;
}

template <typename T, size_t ValuesCount>
inline void Write(AnyPtr address, const T (&values)[ValuesCount]) noexcept {
  uintptr_t addressValue = address.value;

  for (size_t i = 0; i < ValuesCount; ++i, ++addressValue)
    Write(addressValue, values[i]);
}

template <typename T>
inline T Read(ConstAnyPtr address) noexcept {
  return address.get<T>()[0];
}

inline void NOP(AnyPtr address, size_t size) noexcept {
  Fill<uint8_t>(address, size, 0x90);
}

// Returns false if memory in unreadable (segmentation fault)
bool TryRead(ConstAnyPtr address, AnyPtr destination, size_t bytesCount);

template <typename T>
[[nodiscard]] inline std::optional<T> TryRead(ConstAnyPtr address) {
  T result;
  return TryRead(address, &result, sizeof(T)) ? result : std::nullopt;
}

} // namespace Memory
