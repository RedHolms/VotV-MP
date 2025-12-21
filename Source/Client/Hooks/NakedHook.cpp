#include "Hooks/NakedHook.hpp"

#include "Hooks/Relays.hpp"
#include "Hooks/hde/hde32.h"

uintptr_t __cdecl Hooks::NakedRelay(NakedHook* hook) {
  auto returnAddress = hook->returnAddress;

  auto& cb = hook->callback;
  if (cb)
    cb(*hook);

  if (hook->returnAddress != returnAddress) {
    returnAddress = hook->returnAddress;

    auto hookAddress = hook->targetAddress;
    auto hookSize = hook->m_hookSize;
    auto& originalCode = hook->m_originalCode;

    if (returnAddress > hookAddress && returnAddress - hookAddress <= hookSize) {
      uintptr_t result = 0;

      auto m = returnAddress - hookAddress < hookSize ? returnAddress - hookAddress : hookSize;

      for (auto i = 0u; i < m;) {
        hde32s op;
        size_t opSize = hde32_disasm(originalCode.get() + i, &op);

        if (op.flags & F_ERROR)
          return 0;

        if (((op.opcode & 0xF0) == 0x70) || // one byte jump
            ((op.opcode & 0xFC) == 0xE0) || // LOOPNZ/LOOPZ/LOOP/JECXZ
            ((op.opcode2 & 0xF0) == 0x80)) {
          result += sizeof(JCC_REL);
        }
        else if ((op.opcode & 0xFD) == 0xE9) {
          result += sizeof(JMP_REL);
        }
        else if (op.opcode == 0xE8) {
          result += sizeof(JMP_REL);
        }
        else {
          result += opSize;
        }

        i += opSize;
      }

      return result;
    }

    return ~0u;
  }

  return 0;
}
