#include "Common.hpp"

#include "Logging/Asserts.hpp"
#include <hde64.h>

size_t HooksCommon::CreateTrampoline(
  uintptr_t targetAddress,
  uint8_t*& codePtr,
  bool jumpBack
) noexcept {
  JMP_ABS jmp = {
    0xFF,
    0x25,
    0x00000000,           // FF25 00000000: JMP [RIP+6]
    0x0000000000000000ULL // Absolute destination address
  };
  JCC_ABS jcc = {
    0x70,
    0x0E, // 7* 0E:         J** +16
    0xFF,
    0x25,
    0x00000000,           // FF25 00000000: JMP [RIP+6]
    0x0000000000000000ULL // Absolute destination address
  };

  hde64s hs;
  void* opCopySrc = nullptr;
  size_t opCopySize;
  uintptr_t currentAddr = targetAddress;
  bool finished = false;
  uintptr_t maxJmpRef = 0;
  uint8_t instBuffer[16];

  while (!finished) {
    hs = { 0 };

    opCopySrc = reinterpret_cast<void*>(currentAddr);
    opCopySize = hde64_disasm(opCopySrc, &hs);
    if ((hs.flags & F_ERROR) != 0)
      return 0;

    if (currentAddr - targetAddress >= sizeof(JMP_REL)) {
      // We have enough space to inject the jump at "targetAddress"

      if (jumpBack) {
        // Jump to the continuation of the original function
        *codePtr++ = 0xFF;
        *codePtr++ = 0x25;
        *reinterpret_cast<uint32_t*>(codePtr) = 0x00000000;
        codePtr += 4;
        *reinterpret_cast<uintptr_t*>(codePtr) = currentAddr;
        codePtr += 8;
      }

      break;
    }

    // Copy or transform instructions

    if ((hs.modrm & 0xC7) == 0x05) {
      // Instructions using RIP relative addressing. (ModR/M = 00???101B)

      memcpy(instBuffer, opCopySrc, opCopySize);
      opCopySrc = instBuffer;

      // Relative address is stored at (instruction length - immediate value length - 4)
      uint32_t* relAddrPtr =
        reinterpret_cast<uint32_t*>(instBuffer + hs.len - ((hs.flags & 0x3C) >> 2) - 4);

      auto value = currentAddr + static_cast<int32_t>(hs.disp.disp32);

      *relAddrPtr = static_cast<uint32_t>(value - reinterpret_cast<uintptr_t>(codePtr));

      // Complete the function if JMP (FF /4).
      if (hs.opcode == 0xFF && hs.modrm_reg == 4)
        finished = true;
    }
    else if (hs.opcode == 0xE8) {
      // Relative Call

      uintptr_t destination = currentAddr + static_cast<int32_t>(hs.imm.imm32) + hs.len;

      jmp.address = destination;
      opCopySrc = &jmp;
      opCopySize = sizeof(jmp);
    }
    else if ((hs.opcode & 0xFD) == 0xE9) {
      // Relative jmp

      uintptr_t destination = currentAddr + hs.len;
      if (hs.opcode == 0xEB) // short jump
        destination += static_cast<int8_t>(hs.imm.imm8);
      else
        destination += static_cast<int32_t>(hs.imm.imm32);

      if (targetAddress <= destination && destination < (targetAddress + sizeof(JMP_REL))) {
        if (maxJmpRef < destination)
          maxJmpRef = destination;
      }
      else {
        jmp.address = destination;
        opCopySrc = &jmp;
        opCopySize = sizeof(jmp);

        // Exit the function if it is not in the branch.
        finished = targetAddress >= maxJmpRef;
      }
    }
    else if (((hs.opcode & 0xF0) == 0x70) || // one byte jump
             ((hs.opcode & 0xFC) == 0xE0) || // LOOPNZ/LOOPZ/LOOP/JECXZ
             ((hs.opcode2 & 0xF0) == 0x80)) {
      // Conditional relative jmp

      uintptr_t destination = currentAddr + hs.len;

      if ((hs.opcode & 0xF0) == 0x70     // Jcc
          || (hs.opcode & 0xFC) == 0xE0) // LOOPNZ/LOOPZ/LOOP/JECXZ
        destination += static_cast<int8_t>(hs.imm.imm8);
      else
        destination += static_cast<int32_t>(hs.imm.imm32);

      // Simply copy an internal jump.
      if (targetAddress <= destination && destination < (targetAddress + sizeof(JMP_REL))) {
        if (maxJmpRef < destination)
          maxJmpRef = destination;
      }
      else if ((hs.opcode & 0xFC) == 0xE0) {
        // LOOPNZ/LOOPZ/LOOP/JCXZ/JECXZ to the outside are not supported.
        ASSERT(false && "Todo");
        return 0;
      }
      else {
        uint8_t cond = ((hs.opcode != 0x0F ? hs.opcode : hs.opcode2) & 0x0F);
        // Invert the condition in x64 mode to simplify the conditional jump logic.
        jcc.opcode = 0x71 ^ cond;
        jcc.address = destination;
        opCopySrc = &jcc;
        opCopySize = sizeof(jcc);
      }
    }
    else if ((hs.opcode & 0xFE) == 0xC2) {
      // RET
      finished = (currentAddr >= maxJmpRef);
    }

    memcpy(codePtr, opCopySrc, opCopySize);
    codePtr += opCopySize;
    currentAddr += hs.len;
  }

  return currentAddr - targetAddress;
}

static uintptr_t findPrevFreePage(uintptr_t from, uintptr_t to, uintptr_t granularity) noexcept {
  to -= to % granularity; // alignment
  to -= granularity;

  while (from < to) {
    MEMORY_BASIC_INFORMATION mbi;

    if (VirtualQuery(reinterpret_cast<void*>(to), &mbi, sizeof(mbi)) == 0)
      break;

    if (mbi.State == MEM_FREE)
      return to;

    if (reinterpret_cast<uintptr_t>(mbi.AllocationBase) < granularity)
      break;

    to = reinterpret_cast<uintptr_t>(mbi.AllocationBase) - granularity;
  }

  return 0;
}

static uintptr_t findNextFreePage(uintptr_t from, uintptr_t to, uintptr_t granularity) noexcept {
  from -= from % granularity; // alignment
  from += granularity;

  while (from <= to) {
    MEMORY_BASIC_INFORMATION mbi;

    if (VirtualQuery(reinterpret_cast<void*>(from), &mbi, sizeof(mbi)) == 0)
      break;

    if (mbi.State == MEM_FREE)
      return from;

    if (reinterpret_cast<uintptr_t>(mbi.AllocationBase) < granularity)
      break;

    from = reinterpret_cast<uintptr_t>(mbi.AllocationBase) + mbi.RegionSize;

    from += granularity - 1;
    from -= from % granularity;
  }

  return 0;
}

void* HooksCommon::AllocatePageNear(uintptr_t address, size_t range) noexcept {
  constexpr auto PAGE_SIZE = 4096;

  SYSTEM_INFO si;
  GetSystemInfo(&si);

  uintptr_t minAddress = reinterpret_cast<uintptr_t>(si.lpMinimumApplicationAddress);
  uintptr_t maxAddress = reinterpret_cast<uintptr_t>(si.lpMaximumApplicationAddress);

  if (range <= address && minAddress < address - range)
    minAddress = address - range;

  if (address + range <= maxAddress)
    maxAddress = address + range;

  // Make room for one page
  maxAddress -= PAGE_SIZE - 1;

  auto tryAllocateByLowerBound = [&] {
    void* result = nullptr;

    uintptr_t alloc = address;
    while (minAddress <= alloc) {
      alloc = findPrevFreePage(minAddress, alloc, si.dwAllocationGranularity);
      if (alloc == 0)
        break; // no more pages

      result = VirtualAlloc(
        reinterpret_cast<void*>(alloc), PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
      );

      if (result != nullptr)
        break; // successfully allocated
    }

    return result;
  };

  auto tryAllocateByUpperBound = [&] {
    void* result = nullptr;

    uintptr_t alloc = address;
    while (alloc <= maxAddress) {
      alloc = findNextFreePage(alloc, maxAddress, si.dwAllocationGranularity);
      if (alloc == 0)
        break; // no more pages

      result = VirtualAlloc(
        reinterpret_cast<void*>(alloc), PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE
      );

      if (result != nullptr)
        break; // successfully allocated
    }

    return result;
  };

  void* result = tryAllocateByLowerBound();
  if (result != nullptr)
    return result;

  return tryAllocateByUpperBound();
}