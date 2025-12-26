#pragma once

namespace FunctionHooks {

template <typename T>
struct _NormalizeFunc;

template <typename Ret, typename... Args>
struct _NormalizeFunc<Ret (*)(Args...)> {
  using Type = Ret(Args...);
};

template <typename Ret, class C, typename... Args>
struct _NormalizeFunc<Ret (C::*)(Args...)> {
  using Type = Ret(C*, Args...);
};

template <typename Ret, typename... Args>
struct _NormalizeFunc<Ret(Args...)> {
  using Type = Ret(Args...);
};

template <typename T>
using NormalizeFunc = typename _NormalizeFunc<T>::Type;

struct RuntimeFunctionInfo {
  uint16_t firstFreeSlot; // if more than 3 then no free slots
};

template <typename T>
struct RuntimeFunctionInfoGenerator;

template <typename Ret, typename... Args>
struct RuntimeFunctionInfoGenerator<Ret(Args...)> {
  static consteval RuntimeFunctionInfo Generate() {
    RuntimeFunctionInfo result = {};

    uint8_t slot = 0;
    if constexpr (!std::is_void_v<Ret> && !std::is_fundamental_v<Ret> && !std::is_pointer_v<Ret> &&
                  !std::is_reference_v<Ret>) {
      // If return value cannot be stored in RAX, use first slot for it
      ++slot;
    }

    // Each argument is forced to take only one slot (all structs are passed by pointer)
    slot += sizeof...(Args);

    result.firstFreeSlot = slot;

    return result;
  }
};

template <typename Ret, typename... Args>
inline constexpr auto _FuncFirstFreeSlot =
  RuntimeFunctionInfoGenerator<Ret(Args...)>::Generate().firstFreeSlot;

template <typename Hook, typename T, typename _Enable = void>
struct RelayGenerator;

template <typename Hook, typename Ret, typename... Args>
struct RelayGenerator<Hook, Ret(Args...), std::enable_if_t<_FuncFirstFreeSlot<Ret, Args...> <= 3>> {
  static Ret Relay(Args... args, Hook* hook) {
    if (hook->Callback)
      return hook->Callback(args...);
    else
      return hook->GetTrampoline()(args...);
  }
};

template <typename Hook, typename Ret, typename HeadArgsTuple, typename TailArgsTuple>
struct RelayGeneratorHeadTail;

template <typename Hook, typename Ret, typename... HeadArgs, typename... TailArgs>
struct RelayGeneratorHeadTail<Hook, Ret, std::tuple<HeadArgs...>, std::tuple<TailArgs...>> {
  // "Head" in this context is arguments that go in registers
  // "Tail" is arguments that go on the stack

  static Ret Relay(HeadArgs... head, Hook* hook, void* _dummy1, TailArgs... tail) {
    if (hook->Callback)
      return hook->Callback(head..., tail...);
    else
      return hook->GetTrampoline()(head..., tail...);
  }
};

template <typename Tuple, size_t Start, typename Sequence>
struct TupleSliceImpl;

template <typename Tuple, size_t Start, size_t... Is>
struct TupleSliceImpl<Tuple, Start, std::index_sequence<Is...>> {
  using Slice = std::tuple<std::tuple_element_t<Is + Start, Tuple>...>;
};

template <typename Tuple, size_t Start, size_t Count>
struct TupleSlice : TupleSliceImpl<Tuple, Start, std::make_index_sequence<Count>> {};

template <typename Hook, typename Ret, typename... Args>
struct MakeHeadTailRelayGen {
  static constexpr size_t HeadSize = !std::is_void_v<Ret> && !std::is_fundamental_v<Ret> &&
                                         !std::is_pointer_v<Ret> && !std::is_reference_v<Ret>
                                       ? 3 // one slot occupied by return value
                                       : 4;

  using Head = TupleSlice<std::tuple<Args...>, 0, HeadSize>::Slice;
  using Tail = TupleSlice<std::tuple<Args...>, HeadSize, sizeof...(Args) - HeadSize>::Slice;

  using Generator = RelayGeneratorHeadTail<Hook, Ret, Head, Tail>;
};

template <typename Hook, typename Ret, typename... Args>
struct RelayGenerator<
  Hook,
  Ret(Args...),
  std::enable_if_t<((_FuncFirstFreeSlot<Ret, Args...>) > 3)>
> : MakeHeadTailRelayGen<Hook, Ret, Args...>::Generator {};

struct Impl {
  bool installed = false;
  uint8_t* genPage = nullptr;
  void* relayJumper = nullptr;
  void* trampoline = nullptr;
  size_t hookSize = 0;
  std::unique_ptr<uint8_t[]> originalCode;

  ~Impl();

  void Install(
    uintptr_t funcAddress,
    RuntimeFunctionInfo const& funcInfo,
    void* relayAddr,
    void* relayPayload
  ) noexcept;

  void Remove(uintptr_t funcAddress) noexcept;

  void GenerateCode(
    uintptr_t funcAddress,
    RuntimeFunctionInfo const& funcInfo,
    void* relayAddr,
    void* relayPayload
  ) noexcept;
};

} // namespace FunctionHooks
