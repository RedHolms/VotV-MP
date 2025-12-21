#pragma once

#include <stdint.h>

#pragma pack(push, 1)

class CPUFlags {
public:
  uintptr_t CF : 1;

private:
  uintptr_t reserved1 : 1;

public:
  uintptr_t PF : 1;

private:
  uintptr_t reserved2 : 1;

public:
  uintptr_t AF : 1;

private:
  uintptr_t reserved3 : 1;

public:
  uintptr_t ZF : 1;
  uintptr_t SF : 1;
  uintptr_t TF : 1;
  uintptr_t IF : 1;
  uintptr_t DF : 1;
  uintptr_t OF : 1;
  uintptr_t IOPL : 2;
  uintptr_t NT : 1;

private:
  uintptr_t reserved4 : 1;

public:
  uintptr_t RF : 1;
  uintptr_t VM : 1;
  uintptr_t AC : 1;
  uintptr_t VIF : 1;
  uintptr_t VIP : 1;
  uintptr_t ID : 1;

private:
  uintptr_t reserved5 : 10;
};

struct CPUContext {
  uintptr_t edi;
  uintptr_t esi;
  uintptr_t ebp;
  uintptr_t esp;
  uintptr_t ebx;
  uintptr_t edx;
  uintptr_t ecx;
  uintptr_t eax;
  CPUFlags* flags;
};

#pragma pack(pop)
