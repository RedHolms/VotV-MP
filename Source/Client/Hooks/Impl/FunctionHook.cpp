#include "FunctionHook.hpp"

#include "Client/Hooks/Impl/Common.hpp"
#include "Logging/Asserts.hpp"

FunctionHooks::Impl::~Impl() {
  if (genPage != nullptr) {
    _aligned_free(genPage);
  }
}

void FunctionHooks::Impl::Install(
  uintptr_t funcAddress,
  RuntimeFunctionInfo const& funcInfo,
  void* relayAddr,
  void* relayPayload
) noexcept {
  if (installed)
    return;

  void* funcPtr = reinterpret_cast<void*>(funcAddress);

  if (!genPage) {
    GenerateCode(funcAddress, funcInfo, relayAddr, relayPayload);

    originalCode = std::make_unique<uint8_t[]>(hookSize);
    memcpy(originalCode.get(), funcPtr, hookSize);
  }

  DWORD previousProtection;
  VirtualProtect(funcPtr, hookSize, PAGE_EXECUTE_READWRITE, &previousProtection);

#pragma pack(push, 1)
  struct Patch {
    uint8_t opcode;
    uint32_t operand;
  };
#pragma pack(pop)

  auto patch = static_cast<Patch*>(funcPtr);
  if (patch->opcode != 0xE8)
    patch->opcode = 0xE9;
  patch->operand = reinterpret_cast<uintptr_t>(relayJumper) - funcAddress - 5;

  auto nopsCount = hookSize - sizeof(Patch);
  if (nopsCount > 0)
    memset(reinterpret_cast<void*>(funcAddress + sizeof(Patch)), 0x90, nopsCount);

  // Restore
  VirtualProtect(funcPtr, hookSize, previousProtection, &previousProtection);
}

void FunctionHooks::Impl::Remove(uintptr_t funcAddress) noexcept {
  if (!installed)
    return;

  // TODO
}

void FunctionHooks::Impl::GenerateCode(
  uintptr_t funcAddress,
  RuntimeFunctionInfo const& funcInfo,
  void* relayAddr,
  void* relayPayload
) noexcept {
  // Assume page size is 4096 bytes
  genPage = static_cast<uint8_t*>(HooksCommon::AllocatePageNear(funcAddress));
  ASSERT(genPage != nullptr && "Failed to allocate page near hooked function");

  {
    DWORD _unused;
    VirtualProtect(genPage, 4096, PAGE_EXECUTE_READWRITE, &_unused);
  }

  uint8_t* codePtr = genPage;

  // Relay jumper
  // We need to push relayPayload as the last argument and then redirect call to relayAddr
  relayJumper = codePtr;

  // POP return address to RAX
  *codePtr++ = 0x58;

  if (funcInfo.firstFreeSlot > 3) {
    // Put payload on stack
    // Also put dummy pointer to keep 16 bytes alignment

    uintptr_t payloadInt = reinterpret_cast<uintptr_t>(relayPayload);

    // Allocate 16 more bytes on stack
    *codePtr++ = 0x48;
    *codePtr++ = 0x83;
    *codePtr++ = 0xec;
    *codePtr++ = 0x10;

    // First 32(0x20) bytes from RSP will be used for shadow space
    // Payload must go at [RSP+0x20]
    // [RSP+0x28] is one of our dummy pointer, so just ignore it

    *codePtr++ = 0xc7; // mov dword ptr [rsp+
    *codePtr++ = 0x44;
    *codePtr++ = 0x24;
    *codePtr++ = 0x20;                                               // offset
    *reinterpret_cast<uint32_t*>(codePtr) = payloadInt & 0xFFFFFFFF; // value (low dword)
    codePtr += 4;

    *codePtr++ = 0xc7; // mov dword ptr [rsp+
    *codePtr++ = 0x44;
    *codePtr++ = 0x24;
    *codePtr++ = 0x24;                                        // offset
    *reinterpret_cast<uint32_t*>(codePtr) = payloadInt >> 32; // value (high dword)
    codePtr += 4;
  }
  else {
    // Put payload in register
    switch (funcInfo.firstFreeSlot) {
      case 1:
        *codePtr++ = 0x48; // RDX
        *codePtr++ = 0xBA;
        break;
      case 2:
        *codePtr++ = 0x49; // R8
        *codePtr++ = 0xB8;
        break;
      case 3:
        *codePtr++ = 0x49; // R9
        *codePtr++ = 0xB9;
        break;
    }

    *reinterpret_cast<void**>(codePtr) = relayPayload;
    codePtr += 8;
  }

  // Restore return address
  *codePtr++ = 0x50;

  // Jump to relay
  *codePtr++ = 0xFF;
  *codePtr++ = 0x25;
  *reinterpret_cast<uint32_t*>(codePtr) = 0x00000000;
  codePtr += 4;
  *reinterpret_cast<void**>(codePtr) = relayAddr;
  codePtr += 8;

  // Trampoline
  trampoline = codePtr;
  hookSize = HooksCommon::CreateTrampoline(funcAddress, codePtr);
}
