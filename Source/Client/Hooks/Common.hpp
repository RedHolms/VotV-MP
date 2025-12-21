#pragma once

#include "Hooks/xbyak/xbyak.h"

namespace Hooks {

template <typename LeftTuple, typename RightTuple>
struct _TuplesConcat;

template <typename... LeftArgs, typename... RightArgs>
struct _TuplesConcat<std::tuple<LeftArgs...>, std::tuple<RightArgs...>> {
  using Result = std::tuple<LeftArgs..., RightArgs...>;
};

template <typename LeftTuple, typename RightTuple>
using TuplesConcat = typename _TuplesConcat<LeftTuple, RightTuple>::Result;

/// ============================================================================

enum CallConv {
  CallConv__cdecl,
  CallConv__fastcall,
  CallConv__thiscall,
  CallConv__stdcall
};

/// ============================================================================

template <typename FunctionPtr>
struct FunctionTraits;

#define FUNC_TRAITS_FOR_CCONV(conv)                                                                \
  template <typename Ret, typename... Args>                                                        \
  struct FunctionTraits<Ret(conv*)(Args...)> {                                                     \
    static constexpr auto ArgsCount = sizeof...(Args);                                             \
    static constexpr auto Convention = CallConv##conv;                                             \
    using ArgsType = std::tuple<Args...>;                                                          \
    using ReturnType = Ret;                                                                        \
  }

FUNC_TRAITS_FOR_CCONV(__cdecl);
FUNC_TRAITS_FOR_CCONV(__fastcall);
FUNC_TRAITS_FOR_CCONV(__thiscall);
FUNC_TRAITS_FOR_CCONV(__stdcall);

#undef FUNC_TRAITS_FOR_CCONV

template <typename Ret, class Class, typename... Args>
struct FunctionTraits<Ret (Class::*)(Args...)> {
  static constexpr auto ArgsCount = sizeof...(Args) + 1;
  static constexpr auto Convention = CallConv__thiscall;
  using ArgsType = std::tuple<Class*, Args...>;
  using ReturnType = Ret;
};

/// ============================================================================

template <CallConv Conv, typename Ret, typename ArgsTuple>
struct _ConnectFunctionPtr;

#define CONNECT_FUNCTION_PTR_FOR_CONV(conv)                                                        \
  template <typename Ret, typename... Args>                                                        \
  struct _ConnectFunctionPtr<CallConv##conv, Ret, std::tuple<Args...>> {                           \
    using Result = Ret(conv*)(Args...);                                                            \
  }

CONNECT_FUNCTION_PTR_FOR_CONV(__cdecl);
CONNECT_FUNCTION_PTR_FOR_CONV(__fastcall);
CONNECT_FUNCTION_PTR_FOR_CONV(__thiscall);
CONNECT_FUNCTION_PTR_FOR_CONV(__stdcall);

#undef CONNECT_FUNCTION_PTR_FOR_CONV

template <CallConv Conv, typename Ret, typename ArgsTuple>
using ConnectFunctionPtr = typename _ConnectFunctionPtr<Conv, Ret, ArgsTuple>::Result;

/// ============================================================================

template <typename T, typename _Enable = void>
struct _ConvertHookedArg {
  using Result = std::add_lvalue_reference_t<T>;

  using _Ignored = _Enable;
};

template <typename T>
struct _ConvertHookedArg<T, std::enable_if_t<std::is_reference_v<T>>> {
  using Result =
    std::add_lvalue_reference_t<std::add_pointer_t<std::remove_cv_t<std::remove_reference_t<T>>>>;
};

template <typename ArgsTuple>
struct _ConvertHookedArgs;

template <typename... ArgsT>
struct _ConvertHookedArgs<std::tuple<ArgsT...>> {
  using Result = std::tuple<typename _ConvertHookedArg<ArgsT>::Result...>;
};

// Convert all references to pointers & then add lvalue references to all args
template <typename ArgsTuple>
using ConvertHookedArgs = typename _ConvertHookedArgs<ArgsTuple>::Result;

/// ============================================================================

template <typename Ret, typename ArgsTuple>
struct _ConnectFunction;

template <typename Ret, typename... Args>
struct _ConnectFunction<Ret, std::tuple<Args...>> {
  using Result = Ret(Args...);
};

template <typename Ret, typename ArgsTuple>
using ConnectFunction = typename _ConnectFunction<Ret, ArgsTuple>::Result;

/// ============================================================================

#pragma pack(push, 1)

struct JCC_ABS {
  uint8_t opcode; // 7* 0E:         J** +16
  uint8_t dummy0;
  uint8_t dummy1; // FF25 00000000: JMP [+6]
  uint8_t dummy2;
  uint32_t dummy3;
  uint64_t address; // Absolute destination address
};

struct CALL_ABS {
  uint8_t opcode0; // FF15 00000002: CALL [+6]
  uint8_t opcode1;
  uint32_t dummy0;
  uint8_t dummy1; // EB 08:         JMP +10
  uint8_t dummy2;
  uint64_t address; // Absolute destination address
};

struct JMP_ABS {
  uint8_t opcode0; // FF25 00000000: JMP [+6]
  uint8_t opcode1;
  uint32_t dummy;
  uint64_t address; // Absolute destination address
};

typedef struct {
  uint8_t opcode;   // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
  uint32_t operand; // Relative destination address
} JMP_REL, CALL_REL;

struct JCC_REL {
  uint8_t opcode0; // 0F8* xxxxxxxx: J** +6+xxxxxxxx
  uint8_t opcode1;
  uint32_t operand; // Relative destination address
};

#pragma pack(pop) // pack(push, 1)

/// ============================================================================

bool IsCodeExecutable(void const* address);
bool TryCreateTrampoline(uintptr_t hookAddress, Xbyak::CodeGenerator& codeGen, bool naked = false);
size_t DetectHookSize(uintptr_t hookAddress);

inline uintptr_t GetRelativeAddress(uintptr_t dest, uintptr_t src, size_t opLen = 5) {
  return dest - src - opLen;
}

inline uintptr_t GetAbsoluteAddress(uintptr_t RIP, uintptr_t relative, size_t opLen = 5) {
  return RIP + static_cast<intptr_t>(relative) + opLen;
}

} // namespace Hooks
