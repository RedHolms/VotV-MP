#pragma once

namespace Game {

extern uintptr_t ImageBase;

using Symbol = std::array<char, 256>;

template <Symbol Name>
struct TypedOffsetForSymbol_Wrapper;

template <Symbol Name>
constexpr inline auto OffsetOfSymbol = TypedOffsetForSymbol_Wrapper<Name>::Offset;

template <Symbol Name>
using TypeOfSymbol = typename TypedOffsetForSymbol_Wrapper<Name>::Type;

} // namespace Game

#define DEFINE_SYMBOL(Name, Off, ...)                                                              \
  template <>                                                                                      \
  struct Game::TypedOffsetForSymbol_Wrapper<{ Name }> {                                            \
    using Type = std::type_identity_t<__VA_ARGS__>;                                                \
    static constexpr uintptr_t Offset = Off;                                                       \
  }

#define SYMBOL_TYPE(Name)   Game::TypeOfSymbol<{ Name }>
#define SYMBOL_OFFSET(Name) Game::OffsetOfSymbol<{ Name }>
#define SYMBOL_ADDR(Name)   (Game::ImageBase + SYMBOL_OFFSET(Name))

namespace Game {

template <Symbol Name, typename... Args>
__forceinline auto CallSymbol(Args&&... args) {
  auto funcPtr = std::bit_cast<SYMBOL_TYPE(Name)>(SYMBOL_ADDR(Name));
  return std::invoke(funcPtr, std::forward<Args>(args)...);
}

} // namespace Game
