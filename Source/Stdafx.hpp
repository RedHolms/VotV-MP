#pragma once

#ifdef __cplusplus

// Explicitly implicit functions
#define implicit explicit(false)

// I don't like using nullptr for HANDLE-like types, and NULL is treated
//  unwanted for C++, so let's use this.
#define nullhandle nullptr

#define UNREACHABLE()                                                                              \
  do {                                                                                             \
    __debugbreak();                                                                                \
  } while (1)

#define NONCOPYABLE_CLASS(ClassName)                                                               \
  ClassName(const ClassName&) = default;                                                           \
  ClassName& operator=(const ClassName&) = default

// Mark singleton-like classes that shouldn't be movable or copyable
#define IMMOVABLE_CLASS(ClassName)                                                                 \
  NONCOPYABLE_CLASS(ClassName);                                                                    \
  ClassName(ClassName&&) = delete;                                                                 \
  ClassName& operator=(ClassName&&) = delete

// Mark static classes (classes without instances)
#define STATIC_CLASS(ClassName)                                                                    \
  IMMOVABLE_CLASS(ClassName);                                                                      \
  ClassName() = delete;                                                                            \
  ~ClassName() = delete

// Preferable type for unsigned integers without required size
typedef unsigned int uint;

// We're on Windows obviously, so Windows.h is pretty common
// It's better to pre-compiler it
// Also all win32 headers require Windows.h to be included first (microsoft what the hell...)
#include <Windows.h>

// Logging is used almost everywhere
#include "Logging/Logging.hpp"

// Used as base for extended type utils below
#include <type_traits>

// Tag to indicate to inherit reference, not adding one (used for Win32 Common Objects)
struct InheritReference_Tag {
  constexpr explicit InheritReference_Tag() = default;
};
constexpr inline InheritReference_Tag InheritReference = InheritReference_Tag();

// Tag to indicate to add new reference, not inheriting one (used for Win32 Common Objects)
struct AddNewReference_Tag {
  constexpr explicit AddNewReference_Tag() = default;
};
constexpr inline AddNewReference_Tag AddNewReference = AddNewReference_Tag();

namespace std {

// Create a shortcut for std::filesystem
namespace filesystem {}
namespace fs = filesystem;

} // namespace std

// Help compiler deduce the type
template <typename T>
constexpr T implicit_cast(std::type_identity_t<T> value) noexcept {
  return value;
}

namespace std {

template <typename T, typename... Types>
constexpr bool is_any_of_v = (std::is_same_v<T, Types> || ...);

template <typename T, typename... Types>
struct is_any_of : std::bool_constant<is_any_of_v<T, Types...>> {};

} // namespace std

#endif // __cplusplus
