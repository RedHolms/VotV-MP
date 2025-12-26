#pragma once

namespace HooksCommon {

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

struct JMP_REL {
  uint8_t opcode;   // E9/E8 xxxxxxxx: JMP/CALL +5+xxxxxxxx
  uint32_t operand; // Relative destination address
};

using CALL_REL = JMP_REL;

struct JCC_REL {
  uint8_t opcode0; // 0F8* xxxxxxxx: J** +6+xxxxxxxx
  uint8_t opcode1;
  uint32_t operand; // Relative destination address
};

#pragma pack(pop) // pack(push, 1)

/*
 * Generate such assembly at "codePtr" so it does exactly the same thing what code at
 *  "targetAddress" does. Procedure is repeated until there's enough instructions recreated so we
 *  can put relative jump instruction at "targetAddress" (5 bytes).
 * Once code is fully recreated, if "jumpBack" is true, function also generates jump after generated
 *  code that will jump right to the point where original function (at "targetAddress") continues.
 * This operation creates such code, that we can call "codePtr" exactly how we may call original
 *   function.
 *
 * Returns amount of bytes of code recreated.
 *  Note that if returned value is less than 5 it means that valid trampoline couldn't be created.
 * Returns 0 at error.
 */
size_t CreateTrampoline(uintptr_t targetAddress, uint8_t*& codePtr, bool jumpBack = true) noexcept;

/*
 * Try to allocate memory page (4096 bytes) in specified range(default to 1gb) near to "address"
 */
void* AllocatePageNear(uintptr_t address, size_t range = 0x40000000) noexcept;

} // namespace HooksCommon
