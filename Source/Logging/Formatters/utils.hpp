#pragma once

#include <type_traits>

#define BEGIN_ENUM_FORMATTER(EnumName)                                                             \
  template <>                                                                                      \
  struct fmt::formatter<EnumName> {                                                                \
    constexpr auto parse(format_parse_context& ctx) {                                              \
      return ctx.begin();                                                                          \
    }                                                                                              \
    template <typename FormatContext>                                                              \
    auto format(EnumName value, FormatContext& ctx) const {                                        \
      switch (value) {

#define ENUM_FORMATTER_CASE(Case)                                                                  \
  case Case:                                                                                       \
    return fmt::format_to(ctx.out(), #Case);

#define END_ENUM_FORMATTER(EnumName)                                                               \
      } /* switch (value) */                                                                       \
      return format_to(                                                                            \
        ctx.out(),                                                                                 \
        "(" #EnumName "){}",                                                                       \
        static_cast<std::underlying_type_t<EnumName>>(value)                                       \
      );                                                                                           \
    } /* auto format(...) const */                                                                 \
  }; /* struct fmt::formatter<EnumName> */
