#pragma once

#include "Client/Hooks/Common.hpp"

class NakedHook;

namespace Hooks {

template <typename Callback, class HookClass, typename Ret, class... Args>
__forceinline Ret RelayBody(Callback& cb, HookClass* hook, Args... args) {
  if (cb)
    return cb(hook->GetTrampoline(), args...);

  return hook->GetTrampoline()(args...);
}

template <class HookClass, CallConv Convention, typename Ret, class ArgsTuple>
struct RelayGenerator;

#define RELAY_GEN_FOR_CCONV(conv)                                                                  \
  template <class HookClass, typename Ret, typename... Args>                                       \
  struct RelayGenerator<HookClass, CallConv##conv, Ret, std::tuple<Args...>> {                     \
    static Ret conv Relay(HookClass* hook, Args... args) {                                         \
      auto& cb = hook->callback;                                                                   \
      return RelayBody<decltype(cb), HookClass, Ret, Args...>(cb, hook, args...);                  \
    }                                                                                              \
  }

RELAY_GEN_FOR_CCONV(__cdecl);
RELAY_GEN_FOR_CCONV(__stdcall);
RELAY_GEN_FOR_CCONV(__thiscall);

#undef RELAY_GEN_FOR_CCONV

template <class HookClass, typename Ret, typename... Args>
struct RelayGenerator<HookClass, CallConv__fastcall, Ret, std::tuple<Args...>> {
  // Fastcall puts first two arguments of SIMPLE types in registers
  // But structures will go on stack well
  struct ForcedStackValue {
    HookClass* hook;
  };

  static Ret __fastcall Relay(ForcedStackValue packed, Args... args) {
    auto [hook] = packed;
    auto& cb = hook->callback;
    return RelayBody<decltype(cb), HookClass, Ret, Args...>(cb, hook, args...);
  }
};

///=============================================================================

uintptr_t __cdecl NakedRelay(NakedHook* hook);

} // namespace Hooks
