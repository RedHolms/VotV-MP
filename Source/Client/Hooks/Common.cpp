#include "Common.hpp"

#include "Hooks/hde/hde32.h"

bool Hooks::IsCodeExecutable(const void* address) {
  MEMORY_BASIC_INFORMATION info = {};
  VirtualQuery(address, &info, sizeof(info));
  return info.Protect == PAGE_EXECUTE || info.Protect == PAGE_EXECUTE_READ ||
         info.Protect == PAGE_EXECUTE_READWRITE || info.Protect == PAGE_EXECUTE_WRITECOPY;
}

bool Hooks::TryCreateTrampoline(uintptr_t hookAddress, Xbyak::CodeGenerator& codeGen, bool naked) {
  JMP_REL jmp = {
    0xE9,      // E9 xxxxxxxx: JMP +5+xxxxxxxx
    0x00000000 // Relative destination address
  };
  JCC_REL jcc = {
    0x0F,
    0x80,      // 0F8* xxxxxxxx: J** +6+xxxxxxxx
    0x00000000 // Relative destination address
  };

  size_t trampolineSize = 0;
  size_t opCopySize = 0;
  void* opCopySrc = nullptr;
  uintptr_t currentAddress = hookAddress;
  uintptr_t maxJmpRef = 0;
  bool finished = false;

  while (!finished) {
    hde32s op;
    opCopySize = hde32_disasm(reinterpret_cast<void*>(currentAddress), &op);

    if (op.flags & F_ERROR)
      return false;

    opCopySrc = reinterpret_cast<void*>(currentAddress);
    if (currentAddress - hookAddress >= sizeof(CALL_REL)) {
      if (!naked)
        codeGen.jmp(reinterpret_cast<uint8_t*>(currentAddress));

      break;
    }

    // Relative Call
    if (op.opcode == 0xE8) {
      uintptr_t destination = GetAbsoluteAddress(currentAddress, op.imm.imm32, op.len);

      jmp.operand = GetRelativeAddress(
        destination, reinterpret_cast<uintptr_t>(codeGen.getCurr()), sizeof(jmp)
      );

      opCopySrc = &jmp;
      opCopySize = sizeof(jmp);
    }
    // Relative jmp
    else if ((op.opcode & 0xFD) == 0xE9) {
      uintptr_t destination = currentAddress + op.len;

      if (op.opcode == 0xEB) // is short jump
        destination += static_cast<int8_t>(op.imm.imm8);
      else
        destination += static_cast<int32_t>(op.imm.imm32);

      if (hookAddress <= destination && destination < hookAddress + sizeof(JMP_REL)) {
        if (maxJmpRef < destination)
          maxJmpRef = destination;
      }
      else {
        jmp.operand = GetRelativeAddress(
          destination, reinterpret_cast<uintptr_t>(codeGen.getCurr()), sizeof(jmp)
        );

        opCopySrc = &jmp;
        opCopySize = sizeof(jmp);

        // Exit the function if it is not in the branch.
        finished = (hookAddress >= maxJmpRef);
      }
    }
    // Conditional relative jmp
    else if ((op.opcode & 0xF0) == 0x70 || // one byte jump
             (op.opcode & 0xFC) == 0xE0 || // LOOPNZ/LOOPZ/LOOP/JECXZ
             (op.opcode2 & 0xF0) == 0x80) {
      // two byte jump

      uintptr_t destination = currentAddress + op.len;

      if ((op.opcode & 0xF0) == 0x70       // Jcc
          || (op.opcode & 0xFC) == 0xE0) { // LOOPNZ/LOOPZ/LOOP/JECXZ
        destination += static_cast<int8_t>(op.imm.imm8);
      }
      else {
        destination += static_cast<int32_t>(op.imm.imm32);
      }

      // Simply copy an internal jump.
      if (hookAddress <= destination && destination < (hookAddress + sizeof(JMP_REL))) {
        if (maxJmpRef < destination)
          maxJmpRef = destination;
      }
      else if ((op.opcode & 0xFC) == 0xE0) {
        // LOOPNZ/LOOPZ/LOOP/JCXZ/JECXZ to the outside are not supported.
        return false;
      }
      else {
        uint8_t cond = (op.opcode != 0x0F ? op.opcode : op.opcode2) & 0x0F;

        jcc.opcode1 = 0x80 | cond;
        jcc.operand = GetRelativeAddress(
          destination, reinterpret_cast<uintptr_t>(codeGen.getCurr()), sizeof(jcc)
        );

        opCopySrc = &jcc;
        opCopySize = sizeof(jcc);
      }
    }
    // RET
    else if ((op.opcode & 0xFE) == 0xC2) {
      finished = currentAddress >= maxJmpRef;
    }

    codeGen.db(static_cast<uint8_t*>(opCopySrc), opCopySize);

    trampolineSize += opCopySize;
    currentAddress += op.len;
  }

  if (currentAddress - hookAddress < sizeof(JMP_REL))
    return false;

  return true;
}

size_t Hooks::DetectHookSize(uintptr_t hookAddress) {
  size_t size = 0;
  while (size < 5) {
    hde32s op;
    hde32_disasm(reinterpret_cast<void*>(hookAddress), &op);
    size += op.len;
    hookAddress += op.len;
  }
  return size;
}
