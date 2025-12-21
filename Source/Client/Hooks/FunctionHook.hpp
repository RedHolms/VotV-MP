#pragma once

#include "Hooks/Common.hpp"
#include "Hooks/Relays.hpp"
#include <functional>
#include <memory>

template <typename Target>
class FunctionHook {
public:
  using TargetType = Target;

  using Traits = Hooks::FunctionTraits<Target>;

  using ArgsType = typename Traits::ArgsType;
  using ReturnType = typename Traits::ReturnType;

  using FunctionPointer = Hooks::ConnectFunctionPtr<Traits::Convention, ReturnType, ArgsType>;

  using ConvertedArgs = Hooks::ConvertHookedArgs<ArgsType>;

  using Callback = std::function<Hooks::ConnectFunction<
    ReturnType,
    Hooks::TuplesConcat<std::tuple<FunctionPointer>, ConvertedArgs>>>;

public:
  uintptr_t targetAddress = 0;
  Callback callback;

public:
  constexpr FunctionHook() = default;
  constexpr explicit FunctionHook(uintptr_t targetAddress)
    : targetAddress(targetAddress) {}

  ~FunctionHook() {
    Remove();
  }

public:
  FunctionPointer GetTrampoline() const {
    return static_cast<FunctionPointer>(static_cast<void const*>(m_trampolineGen->getCode()));
  }

  bool Install() {
    if (m_installed)
      return true;

    if (targetAddress == 0 || !Hooks::IsCodeExecutable(reinterpret_cast<void*>(targetAddress)))
      return false;

    if (m_jumpGen == nullptr) {
      static struct JumpAllocator : Xbyak::Allocator {
        uint8_t* alloc(size_t size) override {
          void* ptr = Xbyak::AlignedMalloc(size, Xbyak::inner::ALIGN_PAGE_SIZE);
          DWORD oldProtect;
          VirtualProtect(ptr, size, PAGE_EXECUTE_READWRITE, &oldProtect);
          return static_cast<uint8_t*>(ptr);
        }

        void free(uint8_t* p) override {}

        bool useProtect() const override {
          return false;
        }
      } _Allocator;

      m_jumpGen =
        std::make_unique<Xbyak::CodeGenerator>(Xbyak::DEFAULT_MAX_CODE_SIZE, nullptr, &_Allocator);
      m_trampolineGen = std::make_unique<Xbyak::CodeGenerator>();
    }

    if (!Hooks::TryCreateTrampoline(targetAddress, *m_trampolineGen))
      return false;

    FlushInstructionCache(
      GetCurrentProcess(), m_trampolineGen->getCode(), m_trampolineGen->getSize()
    );

    if (!Patch(true))
      return false;

    return m_installed = true;
  }

  void Remove() {
    if (!m_installed)
      return;

    Patch(false);
    m_installed = false;
  }

private:
  bool Patch(bool enable) {
    if (!enable) {
      if (m_relayJump) {
        // NOP relay jump
        memcpy(&m_relayJumpBytes, m_relayJump, sizeof(m_relayJumpBytes));
        m_jumpGen->rewrite(0, 0x9090909090909090, sizeof(m_relayJumpBytes));
      }
    }
    else /*if (enable)*/ {
#pragma pack(push, 1)
      struct {
        uint8_t opcode;
        uint32_t operand;
      } patch;
#pragma pack(pop)

      if (m_relayJump) {
        // Relay jump already generated, just restore it
        m_jumpGen->rewrite(0, m_relayJumpBytes, 8);
      }
      else {
        // Generate relay jump
        m_hookSize = Hooks::DetectHookSize(targetAddress);
        m_relayJump = GenerateRelayJump();

        void* targetVoidPtr = reinterpret_cast<void*>(targetAddress);

        DWORD oldProtection = 0;
        BOOL success =
          VirtualProtect(targetVoidPtr, m_hookSize, PAGE_EXECUTE_READWRITE, &oldProtection);

        if (!success)
          return false;

        m_originalCode = std::make_unique<std::byte[]>(m_hookSize);
        memcpy(m_originalCode.get(), targetVoidPtr, m_hookSize);

        uintptr_t relative =
          Hooks::GetRelativeAddress(reinterpret_cast<uintptr_t>(m_relayJump), targetAddress);

        memcpy(&patch, targetVoidPtr, sizeof(patch));

        if (patch.opcode != 0xE8)
          patch.opcode = 0xE9;
        patch.operand = relative;

        memcpy(targetVoidPtr, &patch, sizeof(patch));

        memset(
          reinterpret_cast<void*>(targetAddress + sizeof(patch)), 0x90, m_hookSize - sizeof(patch)
        );

        FlushInstructionCache(GetCurrentProcess(), targetVoidPtr, m_hookSize);
        success = VirtualProtect(targetVoidPtr, m_hookSize, oldProtection, &oldProtection);

        if (!success)
          return false;
      }
    }

    FlushInstructionCache(GetCurrentProcess(), m_relayJump, m_jumpGen->getSize());

    return true;
  }

  uint8_t const* GenerateRelayJump() {
    using namespace Xbyak::util;

    auto hookAddress = targetAddress;

    Xbyak::Label UserCode;

    // this jump gets nopped when hook.remove() is called
    m_jumpGen->jmp(UserCode, Xbyak::CodeGenerator::LabelType::T_NEAR);
    m_jumpGen->nop(3);

    // create trampoline
    Hooks::TryCreateTrampoline(hookAddress, *m_jumpGen);
    m_jumpGen->L(UserCode);

    m_jumpGen->mov(eax, ptr[esp]);
    m_jumpGen->mov(ptr[&m_lastReturnAddress], eax);

    constexpr bool canBePushed = [] {
      if constexpr (Traits::ArgsCount > 0) {
        using first = std::tuple_element_t<0, ArgsType>;
        return std::is_integral_v<first> || std::is_pointer_v<first>;
      }

      return false;
    }();

    constexpr bool isThiscall = Traits::Convention == Hooks::CallConv__thiscall;
    constexpr bool isFastcall = Traits::Convention == Hooks::CallConv__fastcall;

    m_jumpGen->pop(eax);

    if constexpr (!std::is_void_v<ReturnType>) {
      constexpr bool is_fully_nontrivial =
        !std::is_trivial_v<ReturnType> || !std::is_trivially_destructible_v<ReturnType>;

      if constexpr (sizeof(ReturnType) > 8 || is_fully_nontrivial) {
        if constexpr ((isThiscall || isFastcall) &&
                      (sizeof(ReturnType) % 2 != 0 || is_fully_nontrivial)) {
          m_jumpGen->push(reinterpret_cast<std::uintptr_t>(this));
          if constexpr (isThiscall)
            m_jumpGen->push(ecx);
        }
        else {
          m_jumpGen->pop(eax);
          m_jumpGen->push(reinterpret_cast<std::uintptr_t>(this));
          m_jumpGen->push(eax);
          m_jumpGen->mov(eax, ptr[reinterpret_cast<std::uintptr_t>(&m_lastReturnAddress)]);
        }
      }
      else {
        if constexpr (isThiscall) {
          if constexpr (canBePushed)
            m_jumpGen->push(ecx);
        }
        m_jumpGen->push(reinterpret_cast<uintptr_t>(this));
      }
    }
    else {
      if constexpr (isThiscall) {
        if constexpr (canBePushed)
          m_jumpGen->push(ecx);
      }
      m_jumpGen->push(reinterpret_cast<uintptr_t>(this));
    }

    using RelayGen = Hooks::RelayGenerator<FunctionHook, Traits::Convention, ReturnType, ArgsType>;

    void* relayPtr = &RelayGen::Relay;

    if constexpr (Traits::Convention == Hooks::CallConv__cdecl) {
      // call relay for restoring stack pointer after call
      m_jumpGen->call(relayPtr);
      m_jumpGen->add(esp, 4);
      m_jumpGen->jmp(ptr[&m_lastReturnAddress]);
    }
    else {
      m_jumpGen->push(eax);
      m_jumpGen->jmp(relayPtr);
    }

    FlushInstructionCache(GetCurrentProcess(), m_jumpGen->getCode(), m_jumpGen->getSize());

    return m_jumpGen->getCode();
  }

private:
  bool m_installed = false;
  std::unique_ptr<std::byte[]> m_originalCode;
  mutable uintptr_t m_lastReturnAddress = 0;
  size_t m_hookSize = 0;
  uint8_t const* m_relayJump = nullptr;
  uint64_t m_relayJumpBytes = 0;
  std::unique_ptr<Xbyak::CodeGenerator> m_jumpGen;
  std::unique_ptr<Xbyak::CodeGenerator> m_trampolineGen;
};
