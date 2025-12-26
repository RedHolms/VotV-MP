#pragma once

#include "Client/Game/Game.hpp"
#include "Client/Hooks/Impl/FunctionHook.hpp"
#include <functional>

template <Game::Symbol Name>
class SymbolHook {
  IMMOVABLE_CLASS(SymbolHook);

private:
  using FuncType = FunctionHooks::NormalizeFunc<SYMBOL_TYPE(Name)>;

public:
  std::function<FuncType> Callback = {};

public:
  SymbolHook() = default;

  inline ~SymbolHook() noexcept {
    Remove();
  }

public:
  inline void Install() noexcept {
    using FuncInfoGen = FunctionHooks::RuntimeFunctionInfoGenerator<FuncType>;
    using RelayGen = FunctionHooks::RelayGenerator<SymbolHook, FuncType>;

    auto relayPtr = reinterpret_cast<void*>(RelayGen::Relay);
    m_impl.Install(SYMBOL_ADDR(Name), FuncInfoGen::Generate(), relayPtr, this);
  }

  inline void Remove() noexcept {
    m_impl.Remove(SYMBOL_ADDR(Name));
  }

  inline FuncType* GetTrampoline() noexcept {
    // We can't use static_cast here because cast from void* to function pointer witch static cast
    //  is Microsoft extension.
    // ReSharper disable once CppReinterpretCastFromVoidPtr
    return reinterpret_cast<FuncType*>(m_impl.trampoline);
  }

private:
  FunctionHooks::Impl m_impl;
};
