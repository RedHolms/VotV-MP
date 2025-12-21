#pragma once

#include <bit>
#include <optional>
#include <string_view>
#include <kthook/kthook.hpp>

namespace Game {

  extern uintptr_t ImageBase;

  template <typename Decl>
  struct Function {
    using Declaration = Decl;

    consteval Function(std::string_view scaryName, uintptr_t offset)
      : ScaryName(scaryName), Offset(offset) {}

    const std::string_view ScaryName;
    const uintptr_t Offset;

    template <typename... ArgsT>
    constexpr auto operator()(ArgsT&&... args) const {
      // a little bit tricky because we must support member functions
      auto fptr = std::bit_cast<Declaration>(ImageBase + Offset);
      return std::invoke(fptr, std::forward<ArgsT>(args)...);
    }

    constexpr kthook::kthook_simple<Decl>& Hook() const {
      static kthook::kthook_simple<Decl> hookObject { ImageBase + Offset };
      hookObject.install(); // won't install if already installed

      return hookObject;
    }
  };

} // namespace Game
