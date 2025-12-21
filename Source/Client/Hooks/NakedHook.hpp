#pragma once

#include "Hooks/Common.hpp"
#include "Hooks/Context.hpp"
#include "Hooks/Relays.hpp"
#include <functional>
#include <memory>

class NakedHook {
  friend uintptr_t __cdecl Hooks::NakedRelay(NakedHook*);

public:
  using Callback = std::function<void(NakedHook& hook)>;

public:
  uintptr_t targetAddress = 0;
  Callback callback;

  mutable uintptr_t returnAddress = 0;

public:
  constexpr NakedHook() = default;

public:
  CPUContext& GetContext() noexcept {
    return m_context;
  }

  bool Install() {
    if (m_installed)
      return false;
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

    m_installed = true;
    return true;
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

    Xbyak::Label UserCode, ret_addr, jump_in_trampoline, skip_bytes, jump_out;

    // this jump gets nopped when hook.remove() is called
    m_jumpGen->jmp(UserCode, Xbyak::CodeGenerator::LabelType::T_NEAR);
    m_jumpGen->nop(3);

    // create trampoline
    Hooks::TryCreateTrampoline(targetAddress, *m_jumpGen);
    m_jumpGen->L(UserCode);

    m_jumpGen->pushfd();

    m_jumpGen->mov(ptr[&returnAddress], esp);

    // &context.end -> esp
    // pushad -> context.registers
    // memory.esp -> esp
    // esp -> context.esp
    // &label ret_addr  -> eax
    // info.hook_address -> last_return_address
    m_jumpGen->mov(esp, reinterpret_cast<uintptr_t>(&m_context.flags));
    m_jumpGen->pushad();
    m_jumpGen->mov(esp, ptr[&returnAddress]);
    m_jumpGen->mov(ptr[reinterpret_cast<uintptr_t>(&m_context.flags)], esp);
    m_jumpGen->sub(esp, sizeof(CPUFlags));
    m_jumpGen->mov(ptr[reinterpret_cast<uintptr_t>(&m_context.esp)], esp);
    m_jumpGen->add(esp, sizeof(CPUFlags));
    m_jumpGen->mov(eax, ret_addr);

    m_jumpGen->mov(dword[reinterpret_cast<uintptr_t>(&returnAddress)], targetAddress);

    // push this
    // push eax(&label ret_addr)
    m_jumpGen->push(reinterpret_cast<uintptr_t>(this));
    m_jumpGen->push(eax);

    // GOTO callback(call)
    m_jumpGen->jmp(reinterpret_cast<void const*>(&Hooks::NakedRelay));
    m_jumpGen->L(ret_addr);

    // restore stack
    m_jumpGen->add(esp, 0x04);

    // if need to skip trampoline
    m_jumpGen->cmp(eax, ~0u);
    m_jumpGen->je(jump_out);

    // if need to jump inside trampoline
    m_jumpGen->cmp(eax, m_hookSize);
    m_jumpGen->jl(jump_in_trampoline);

    m_jumpGen->L(jump_out);

    // esp -> &context.top
    // context.registers -> popad
    // context.esp -> esp
    // stack -> popfd
    // goto RETURN_ADDR
    m_jumpGen->mov(esp, reinterpret_cast<uintptr_t>(&m_context.edi));
    m_jumpGen->popad();
    m_jumpGen->mov(esp, ptr[reinterpret_cast<uintptr_t>(&m_context.esp)]);
    m_jumpGen->add(esp, sizeof(CPUFlags));
    m_jumpGen->popfd();
    m_jumpGen->jmp(ptr[&returnAddress]);

    m_jumpGen->L(jump_in_trampoline);
    // eax -> esp
    // &label skip_bytes -> eax
    // eax += esp
    // so eax is address inside trampoline
    m_jumpGen->mov(esp, eax);
    m_jumpGen->mov(eax, skip_bytes);
    m_jumpGen->add(eax, esp);

    // eax -> RETURN_ADDR
    // esp -> &context.top
    // popad
    // context.esp -> esp
    // stack -> popfd
    // goto RETURN_ADDR
    m_jumpGen->mov(dword[reinterpret_cast<uintptr_t>(&returnAddress)], eax);
    m_jumpGen->mov(esp, reinterpret_cast<uintptr_t>(&m_context.edi));
    m_jumpGen->popad();
    m_jumpGen->mov(esp, ptr[reinterpret_cast<uintptr_t>(&m_context.esp)]);
    m_jumpGen->add(esp, sizeof(CPUFlags));
    m_jumpGen->popfd();
    m_jumpGen->jmp(ptr[&returnAddress]);
    m_jumpGen->L(skip_bytes);
    Hooks::TryCreateTrampoline(targetAddress, *m_jumpGen);

    FlushInstructionCache(GetCurrentProcess(), m_jumpGen->getCode(), m_jumpGen->getSize());
    return m_jumpGen->getCode();
  }

private:
  bool m_installed = false;
  std::unique_ptr<std::byte[]> m_originalCode;
  size_t m_hookSize = 0;
  uint8_t const* m_relayJump = nullptr;
  uint64_t m_relayJumpBytes = 0;
  std::unique_ptr<Xbyak::CodeGenerator> m_jumpGen;
  std::unique_ptr<Xbyak::CodeGenerator> m_trampolineGen;
  CPUContext m_context;
};
