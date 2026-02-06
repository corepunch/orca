#include <assert.h>
#include <memory.h>

#include "vcore_local.h"

#define REG_SIZE sizeof(uint32_t)
#define REG(vm, reg) vm->r[op.r##reg]
#define BIT_VALUE(value, number) ((value >> number) & 1)

typedef uint32_t (*_ARMPROC)(PVM vm,
                             uint32_t instr,
                             uint32_t Op1,
                             uint32_t Op2);

#define DECL_ARMPROC(NAME)                                                     \
  uint32_t f_##NAME(PVM vm, uint32_t instr, uint32_t Op1, uint32_t Op2)

#define CARRY(vm) (vm->cpsr & CPSR_C ? 1 : 0)

#define VM_ZERO(vm) vm->r[0]
#define REG_INSTR(vm, instr, OFFS) vm->r[(instr >> OFFS) & 0b1111]
#define REG_Rd(vm, instr) REG_INSTR(vm, instr, 12)
#define REG_Rn(vm, instr) REG_INSTR(vm, instr, 16)

DECL_ARMPROC(ADD)
{
  return Op1 + Op2;
}
DECL_ARMPROC(SUB)
{
  return Op1 - Op2;
}
DECL_ARMPROC(RSB)
{
  return Op2 - Op1;
}
DECL_ARMPROC(ADC)
{
  return Op1 + Op2 + CARRY(vm);
}
DECL_ARMPROC(SBC)
{
  return Op1 - Op2 + CARRY(vm);
}
DECL_ARMPROC(RSC)
{
  return Op1 - Op2 + CARRY(vm);
}
DECL_ARMPROC(MUL)
{
  return Op1 * Op2;
}
DECL_ARMPROC(AND)
{
  return Op1 & Op2;
}
DECL_ARMPROC(ORR)
{
  return Op1 | Op2;
}
DECL_ARMPROC(EOR)
{
  return Op1 ^ Op2;
}
DECL_ARMPROC(BIC)
{
  return Op1 & ~Op2;
}
DECL_ARMPROC(MOV)
{
  return Op2;
}
DECL_ARMPROC(MVN)
{
  return ~Op2;
}
DECL_ARMPROC(NOP)
{
  return VM_ZERO(vm);
}

DECL_ARMPROC(ADDS)
{
  uint32_t Rd = f_ADD(vm, instr, Op1, Op2);
  vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
  vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
  vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
  vm->cpsr |= (Rd < Op1 || Rd < Op2) ? CPSR_C : 0;
  vm->cpsr |= ((Op1 ^ Op2) & MSB) && ((Rd ^ Op1) & MSB) ? CPSR_V : 0;
  return Rd;
}

DECL_ARMPROC(SUBS)
{
  uint32_t Rd = f_SUB(vm, instr, Op1, Op2);
  vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
  vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
  vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
  vm->cpsr |= (Op1 >= Op2) ? CPSR_C : 0;
  vm->cpsr |= ((Op1 ^ Op2) & MSB) && ((Rd ^ Op1) & MSB) ? CPSR_V : 0;
  return Rd;
}

DECL_ARMPROC(RSBS)
{
  uint32_t Rd = f_RSB(vm, instr, Op1, Op2);
  vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
  vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
  vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
  vm->cpsr |= (Op2 >= Op1) ? CPSR_C : 0;
  vm->cpsr |= ((Op2 ^ Op1) & MSB) && ((Rd ^ Op2) & MSB) ? CPSR_V : 0;
  return Rd;
}

DECL_ARMPROC(ANDS)
{
  uint32_t Rd = f_AND(vm, instr, Op1, Op2);
  vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
  vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
  vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
  return Rd;
}

DECL_ARMPROC(EORS)
{
  uint32_t Rd = f_EOR(vm, instr, Op1, Op2);
  vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
  vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
  vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
  return Rd;
}

DECL_ARMPROC(CMP)
{
  f_SUBS(vm, instr, Op1, Op2);
  return VM_ZERO(vm);
}

DECL_ARMPROC(CMN)
{
  if (Op2 == -2147483627) {
    int a = 0;
  }
  f_ADDS(vm, instr, Op1, Op2);
  return VM_ZERO(vm);
}

DECL_ARMPROC(TST)
{
  f_ANDS(vm, instr, Op1, Op2);
  return VM_ZERO(vm);
}

DECL_ARMPROC(TEQ)
{
  f_EORS(vm, instr, Op1, Op2);
  return VM_ZERO(vm);
}

lpcString_t names[] = {
  "and", "eor", "sub", "rsb", "add", "adc", "sbc", "rsc", "tst",
  "teq", "cmp", "cmn", "orr", "mov", "bic", "mvn", "nop",
};

_ARMPROC _dp0[] = {
  f_AND, f_EOR, f_SUB, f_RSB, f_ADD, f_ADC, f_SBC, f_RSC, f_TST,
  f_TEQ, f_CMP, f_CMN, f_ORR, f_MOV, f_BIC, f_MVN, f_NOP,
};

_ARMPROC _dp1[] = {
  f_ANDS, f_EORS, f_SUBS, f_RSBS, f_ADDS, f_ADC, f_SBC, f_RSC, f_TST,
  f_TEQ,  f_CMP,  f_CMN,  f_ORR,  f_MOV,  f_BIC, f_MVN, f_NOP,
};

lpcString_t
vm_findsymname(PVM vm, uint32_t pos);

bool_t
_condition(PVM vm, uint32_t cond)
{
  uint32_t f = vm->cpsr;
  switch (cond) {
    case OPCOND_EQ: // Z set, equal
      return (f & CPSR_Z);
    case OPCOND_NE: // Z clear, not equal
      return !(f & CPSR_Z);
    case OPCOND_CS: // C set, unsigned higher or same
      return (f & CPSR_C);
    case OPCOND_CC: // C clear, unsigned lower
      return !(f & CPSR_C);
    case OPCOND_MI: // N set, negative
      return (f & CPSR_N);
    case OPCOND_PL: // N clear, positive or zero
      return !(f & CPSR_N);
    case OPCOND_VS: // V set, overflow
      return (f & CPSR_V);
    case OPCOND_VC: // V clear,no overflow
      return !(f & CPSR_V);
    case OPCOND_HI: // C set and Z clear, unsigned higher
      return (f & CPSR_C) && !(f & CPSR_Z);
    case OPCOND_LS: // C clear or Z set, unsigned lower or same
      return !(f & CPSR_C) || (f & CPSR_Z);
    case OPCOND_GE: // N equals V, greater or equal
      return !(f & CPSR_N) == !(f & CPSR_V);
    case OPCOND_LT: // N not equal to V, less than
      return !(f & CPSR_N) != !(f & CPSR_V);
    case OPCOND_GT: // Z clear AND (N equals V), greater than
      return !(f & CPSR_Z) && (!(f & CPSR_N) == !(f & CPSR_V));
    case OPCOND_LE: // Z set OR (N not equal to V), less than or equal
      return (f & CPSR_Z) || (!(f & CPSR_N) != !(f & CPSR_V));
    case OPCOND_AL: // (ignored), always
    default:
      return 1;
  }
}

uint32_t
_calcshift(PVM vm, uint32_t instr)
{
  uint32_t Rm = instr & 0b1111;
  uint32_t ShiftType = (instr >> 5) & 0b11;
  uint32_t ShiftAmount;
  if (BIT_VALUE(instr, 4)) {
    uint32_t Rs = (instr >> 8) & 0b1111;
    ShiftAmount = vm->r[Rs];
  } else {
    ShiftAmount = (instr >> 7) & 0b11111;
  }
  switch (ShiftType) {
    case 0b00:
      return vm->r[Rm] << ShiftAmount;
    case 0b01:
      return vm->r[Rm] >> ShiftAmount;
    case 0b10:
      return (int)vm->r[Rm] >> ShiftAmount;
    case 0b11:
      return vm->r[Rm] >> ShiftAmount;
  }
  return 0;
}

uint32_t
_calcimmediate(PVM vm, uint32_t instr)
{
  uint32_t Imm = instr & 0xff;
  uint32_t Rotate = ((instr >> 8) & 0xf) << 1;
  return Imm >> Rotate | (Imm << (32 - Rotate));
}

void
exec_dataprocessing(PVM vm, uint32_t instr)
{
  bool_t Immediate = BIT_VALUE(instr, 25);
  bool_t SetFlags = BIT_VALUE(instr, 20);
  OPCODE OpCode = (instr >> 21) & 0xf;
  uint32_t Op = Immediate ? _calcimmediate(vm, instr) : _calcshift(vm, instr);
  uint32_t Rn = REG_Rn(vm, instr);
  REG_Rd(vm, instr) = (SetFlags ? _dp1 : _dp0)[OpCode](vm, instr, Rn, Op);
  if (((instr >> 12) & 0b1111) == PC_REG) {
    vm->location = REG_Rd(vm, instr);
  }
}

static inline uint32_t
_loadptr(PVM vm, int offset)
{
  return *((uint32_t*)(vm->memory + offset));
}

static inline void
_storeptr(PVM vm, int offset, uint32_t value)
{
  *((uint32_t*)(vm->memory + offset)) = value;
}

static inline uint32_t
_offsetptr(uint32_t Rn, uint32_t Offset, bool_t Up)
{
  return Up ? (Rn + Offset) : (Rn - Offset);
}

#define LDR_UP_BIT 23
#define LDR_PREOFFSET_BIT 24
#define LDR_BYTE_BIT 22
#define LDR_WRITEBACK_BIT 21
#define LDR_LOAD_BIT 20

void
exec_datatransfer(PVM vm, uint32_t instr)
{
  bool_t Pre = BIT_VALUE(instr, LDR_PREOFFSET_BIT);
  bool_t Immediate = !BIT_VALUE(instr, 25);
  bool_t Byte = BIT_VALUE(instr, LDR_BYTE_BIT);
  bool_t Up = BIT_VALUE(instr, LDR_UP_BIT);
  uint32_t Rn = REG_Rn(vm, instr);
  uint32_t Offset = Immediate ? instr & 0xfff : _calcshift(vm, instr);
  uint32_t Pointer = _offsetptr(Rn, Offset, Up);
  uint32_t Mask = Byte ? 0xff : 0xffffffff;
  uint32_t Existing = _loadptr(vm, Pre ? Pointer : Rn);

  if (BIT_VALUE(instr, LDR_LOAD_BIT)) {
    REG_Rd(vm, instr) = Existing & Mask;
  } else {
    uint32_t Val = REG_Rd(vm, instr) & Mask;
    _storeptr(vm, Pre ? Pointer : Rn, Val | (Existing & ~Mask));
  }

  if (BIT_VALUE(instr, LDR_WRITEBACK_BIT) || !Pre) {
    REG_Rn(vm, instr) = Pointer;
  }
}

#define LDRSB_IMMEDIATE_BIT 22
#define LDRSB_HALFWORD_BIT 5
#define LDRSB_SIGNED_BIT 6

void
exec_ldrsb(PVM vm, uint32_t instr, uint32_t Rm)
{
  bool_t Pre = BIT_VALUE(instr, LDR_PREOFFSET_BIT);
  bool_t Immediate = BIT_VALUE(instr, LDRSB_IMMEDIATE_BIT);
  bool_t Halfword = BIT_VALUE(instr, LDRSB_HALFWORD_BIT);
  bool_t Signed = BIT_VALUE(instr, LDRSB_SIGNED_BIT);
  uint32_t Rn = REG_INSTR(vm, instr, 16);
  uint32_t Offset = Immediate ? ((instr & 0xf00) >> 4) | (instr & 0xf) : Rm;
  uint32_t Pointer = _offsetptr(Rn, Offset, BIT_VALUE(instr, LDR_UP_BIT));
  uint32_t Mask = Halfword ? 0xffff : 0xff;
  uint32_t Sign = Halfword ? (1 << 15) : (1 << 7);
  uint32_t Existing = _loadptr(vm, Pre ? Pointer : Rn);

  if (BIT_VALUE(instr, LDR_LOAD_BIT)) {
    uint32_t empty = (Existing & Sign) ? ~Mask : 0;
    REG_Rd(vm, instr) = (Signed ? empty : 0) | (Existing & Mask);
  } else {
    uint32_t Val = REG_Rd(vm, instr) & Mask;
    _storeptr(vm, Pre ? Pointer : Rn, Val | (Existing & ~Mask));
  }

  if (BIT_VALUE(instr, LDR_WRITEBACK_BIT) || !Pre) {
    REG_Rn(vm, instr) = Pointer;
  }
}

void
exec_blockdatatransfer(PVM vm, uint32_t instr)
{
  bool_t Load = BIT_VALUE(instr, 20);
  bool_t WriteBack = BIT_VALUE(instr, 21);
  //    bool_t  SetFlags = BIT_VALUE(instr, 22);
  bool_t Up = BIT_VALUE(instr, 23);
  bool_t Pre = BIT_VALUE(instr, 24);
  uint32_t Rn = REG_Rn(vm, instr);
  for (uint32_t i = 0; i <= 0xf; i++) {
    uint32_t j = Up ? i : (0xf - i);
    if (BIT_VALUE(instr, j)) {
      uint32_t Next = Up ? (Rn + REG_SIZE) : (Rn - REG_SIZE);
      if (Load) {
        vm->r[j] = _loadptr(vm, Pre ? Next : Rn);
      } else {
        _storeptr(vm, Pre ? Next : Rn, vm->r[j]);
      }
      Rn = Next;
    }
  }
  if (WriteBack) {
    REG_Rn(vm, instr) = Rn;
  }
}

#define MASK_24BIT 0x00ffffff

void
exec_branchwithlink(PVM vm, uint32_t instr)
{
  bool_t Link = BIT_VALUE(instr, 24);
  bool_t Negative = BIT_VALUE(instr, 23);
  uint32_t Offset = instr & MASK_24BIT;
  if (Link) {
    vm->r[LR_REG] = vm->location;
  }
  if (Negative) {
    vm->location -= (~(Offset - 1)) & MASK_24BIT;
  } else {
    vm->location += Offset;
  }
}

void
exec_branchandexchange(PVM vm, uint32_t instr)
{
  uint32_t Rn = instr & 0xf;
  vm->location = vm->r[Rn];
}

void
exec_mul(PVM vm, uint32_t instr)
{
  bool_t Accumulate = BIT_VALUE(instr, 21);
  bool_t SetFlags = BIT_VALUE(instr, 20);
  uint32_t Rm = REG_INSTR(vm, instr, 0);
  uint32_t Rs = REG_INSTR(vm, instr, 8);
  uint32_t Rn = REG_INSTR(vm, instr, 12);
  if (Accumulate) {
    REG_INSTR(vm, instr, 16) = Rm * Rs + Rn;
  } else {
    REG_INSTR(vm, instr, 16) = Rm * Rs;
  }
  if (SetFlags) {
    uint32_t Rd = REG_INSTR(vm, instr, 16);
    vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
    vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
    vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
    vm->cpsr |= (Rd < Rm || Rd < Rs) ? CPSR_C : 0;
    vm->cpsr |= ((Rm ^ Rs) & MSB) && ((Rd ^ Rm) & MSB) ? CPSR_V : 0;
  }
}

void
exec_umul(PVM vm, uint32_t instr)
{
  typedef long long slong;
  typedef unsigned long long ulong;
  bool_t Accumulate = BIT_VALUE(instr, 21);
  bool_t SetFlags = BIT_VALUE(instr, 20);
  bool_t Signed = BIT_VALUE(instr, 22);
  uint32_t Rm = REG_INSTR(vm, instr, 0);
  uint32_t Rs = REG_INSTR(vm, instr, 8);
  if (Signed) {
    slong result = 0;
    if (Accumulate) {
      uint32_t RdLo = REG_INSTR(vm, instr, 12);
      uint32_t RdHi = REG_INSTR(vm, instr, 16);
      result = (slong)Rm * (slong)Rs + ((slong)RdHi << 32) | RdLo;
    } else {
      result = (slong)Rm * (slong)Rs;
    }
    REG_INSTR(vm, instr, 16) = result >> 32;
    REG_INSTR(vm, instr, 12) = result & 0xffffffff;
  } else {
    ulong result = 0;
    if (Accumulate) {
      uint32_t RdLo = REG_INSTR(vm, instr, 12);
      uint32_t RdHi = REG_INSTR(vm, instr, 16);
      result = (ulong)Rm * (ulong)Rs + ((ulong)RdHi << 32) | RdLo;
    } else {
      result = (ulong)Rm * (ulong)Rs;
    }
    REG_INSTR(vm, instr, 16) = result >> 32;
    REG_INSTR(vm, instr, 12) = result & 0xffffffff;
  }
  assert(!SetFlags);
  if (SetFlags) {
    uint32_t Rd = REG_INSTR(vm, instr, 16);
    vm->cpsr &= ~(CPSR_N | CPSR_Z | CPSR_C | CPSR_V);
    vm->cpsr |= (Rd & MSB) ? CPSR_N : 0;
    vm->cpsr |= (Rd == 0) ? CPSR_Z : 0;
    vm->cpsr |= (Rd < Rm || Rd < Rs) ? CPSR_C : 0;
    vm->cpsr |= ((Rm ^ Rs) & MSB) && ((Rd ^ Rm) & MSB) ? CPSR_V : 0;
  }
}

void
exec_branch_external(PVM vm, uint32_t instr)
{
  uint32_t proc = instr & 0xffff;
  bool_t Link = BIT_VALUE(instr, 19);
  *vm->r = vm->syscall(vm, proc);
  if (!Link) {
    vm->location = vm->r[LR_REG];
  }
}

enum
{
  INSTR_BX,
  INSTR_MUL,
  INSTR_UMUL,
  INSTR_LDRSB,
  INSTR_LDRSBI,
  INSTR_TRAP,
  INSTR_NOP,
  INSTR_LDR,
  INSTR_STR,
  INSTR_LDM,
  INSTR_STM,
  INSTR_BRANCH,
  INSTR_DATAPROC,
};

uint32_t
get_instruction_type(uint32_t instr)
{
  if ((instr & MASK_BX) == OP_BX) {
    return INSTR_BX;
  }
  if ((instr & MASK_MUL) == OP_MUL) {
    return INSTR_MUL;
  }
  if ((instr & MASK_UMUL) == OP_UMUL) {
    return INSTR_UMUL;
  }
  if ((instr & MASK_LDRSB) == OP_LDRSB) {
    return INSTR_LDRSB;
  }
  if ((instr & MASK_LDRSBI) == OP_LDRSBI) {
    return INSTR_LDRSBI;
  }
  if ((instr & MASK_BEXT) == OP_BEXT) {
    return INSTR_TRAP;
  }
  if ((instr & MASK_TRAP) == OP_TRAP) {
    return INSTR_NOP;
  }
  switch ((instr >> 25) & 0b111) {
    case 0b000:
    case 0b001:
      return INSTR_DATAPROC;
    case 0b010:
    case 0b011:
      return BIT_VALUE(instr, LDR_LOAD_BIT) ? INSTR_LDR : INSTR_STR;
    case 0b100:
      return BIT_VALUE(instr, LDR_LOAD_BIT) ? INSTR_LDM : INSTR_STM;
    case 0b101:
      return INSTR_BRANCH;
  }
  Con_Error("Unknown instruction %08x", instr);
  return INSTR_NOP;
}

void
print_symbol_at(PVM vm, uint32_t pc)
{
  char name[256];
  FOR_LOOP(i, vm->num_symbols) {}
}

#define pdebug(...)                                                            \
  if (rr)                                                                      \
  Con_Error(__VA_ARGS__)

lpcString_t cmp_str[] = {
  "EQ", "NE", "CS", "CC", "MI", "PL", "VS", "VC",
  "HI", "LS", "GE", "LT", "GT", "LE", "AL",

};

void
exec_instruction(PVM vm)
{
  extern bool_t rr;

  //    PVMDEBUG dbg = vm->debug;
  //    FOR_LOOP(i, vm->numdebugs) {
  //        dbg = vm->debug+i;
  //        if (dbg->location > 808660) {
  //            break;
  //        }
  //    }
  //    puts(dbg->debug);

  uint32_t instr = *(uint32_t*)(vm->memory + vm->location);
  uint32_t code = get_instruction_type(instr);

  vm->opcount++;
  vm->location += REG_SIZE;
  vm->r[PC_REG] = vm->location + REG_SIZE;

  if (!_condition(vm, instr >> 28))
    return;

  int Rd = (instr >> 12) & 0xf;
  int Rn = (instr >> 16) & 0xf;

  switch (code) {
    case INSTR_BX:
      exec_branchandexchange(vm, instr);
      break;
    case INSTR_MUL:
      exec_mul(vm, instr);
      break;
    case INSTR_UMUL:
      exec_umul(vm, instr);
      break;
    case INSTR_LDRSB:
      exec_ldrsb(vm, instr, REG_INSTR(vm, instr, 0));
      break;
    case INSTR_LDRSBI:
      exec_ldrsb(vm, instr, (instr & 0xff) | ((instr >> 4) & 0xff00));
      break;
    case INSTR_TRAP:
      exec_branch_external(vm, instr);
      break;
    case INSTR_NOP:
      break;
    case INSTR_DATAPROC:
      exec_dataprocessing(vm, instr);
      break;
    case INSTR_LDR:
    case INSTR_STR:
      exec_datatransfer(vm, instr);
      break;
    case INSTR_LDM:
    case INSTR_STM:
      exec_blockdatatransfer(vm, instr);
      break;
    case INSTR_BRANCH:
      exec_branchwithlink(vm, instr);
      break;
  }

  void test_mem(PVM);
  test_mem(vm);
}

void
vm_execute_pc(PVM vm, uint32_t pc)
{
  vm->r[LR_REG] = vm->progsize;
  vm->location = pc;
  while (vm->location < vm->progsize) {
    exec_instruction(vm);
    assert(vm->location != 0xffffffff);
  }
}

lpcString_t
vm_findsymname(PVM vm, uint32_t pos)
{
  PVMSYM sym = vm->symbols;
  FOR_LOOP(i, vm->num_symbols)
  {
    if (sym->position == pos) {
      return sym->name;
    }
    LPSTR ptr = sym->name;
    while (*(ptr++))
      ;
    sym = (PVMSYM)ptr;
  }
  return 0;
}

uint32_t
vm_findsymloc(PVM vm, lpcString_t symbol)
{
  PVMSYM sym = vm->symbols;
  FOR_LOOP(i, vm->num_symbols)
  {
    if (!strcmp(sym->name, symbol)) {
      return sym->position;
    }
    LPSTR ptr = sym->name;
    while (*(ptr++))
      ;
    sym = (PVMSYM)ptr;
  }
  return -1;
}

void
vm_execute(PVM vm, lpcString_t symbol)
{
  uint32_t pc = vm_findsymloc(vm, symbol);
  if (pc != -1) {
    vm_execute_pc(vm, pc);
  }
}

PVM
vm_create(VM_SysCall syscall,
          uint32_t stack_size,
          uint32_t heap_size,
          PVMPROGRAM prog)
{
  byte_t* memory =
    malloc(sizeof(struct VM) + stack_size + heap_size + prog->progsize);
  memset(memory, 0, sizeof(struct VM));
  memcpy(memory + sizeof(struct VM), prog->data, prog->progsize);
  PVM vm = (PVM)memory;
  vm->memory = memory + sizeof(struct VM);
  vm->stacksize = stack_size;
  vm->heapsize = heap_size;
  vm->progsize = prog->progsize;
  vm->syscall = syscall;
  vm->symbols = (PVMSYM)(prog->data + prog->progsize);
  vm->num_symbols = prog->num_symbols;
  vm->r[SP_REG] = stack_size + prog->progsize;
  initialize_memory_manager(
    vm, vm->memory + stack_size + prog->progsize, heap_size);
  return vm;
}

void
vm_attach_debug(PVM vm, PVMDEBUG debugs, uint32_t numdebugs)
{
  vm->debug = debugs;
  vm->numdebugs = numdebugs;
}

void
vm_shutdown(PVM vm)
{
  free(vm);
}

// MemBlock structure to represent a block of allocated memory
typedef struct MemBlock
{
  size_t size;
  bool_t used;
  struct MemBlock *next, *prev;
} MemBlock;

// Function to initialize the memory manager
void
initialize_memory_manager(PVM vm, void* buffer, size_t buffer_size)
{
  memset(buffer, -1, buffer_size);
  // Initialize the linked list with a single MemBlock representing the
  // entire buffer
  vm->head = (MemBlock*)buffer;
  vm->head->size = buffer_size - sizeof(MemBlock);
  vm->head->used = FALSE;
  vm->head->next = NULL;
  vm->head->prev = NULL;
}

// Function to allocate memory from the buffer
void*
vm_allocmem(PVM vm, size_t size)
{
  MemBlock* current = vm->head;
  MemBlock* previous = vm->head;
  //    Con_Error("alloc:: %d", size);
  // Traverse the linked list to find a suitable block
  while (current != NULL) {
    if (!current->used && current->size >= size) {
      // Allocate memory from the current block
      if (current->size > size + sizeof(MemBlock)) {
        // If there's enough space, create a new MemBlock for the
        // remaining block
        MemBlock* new_block =
          (MemBlock*)((LPSTR)current + sizeof(MemBlock) + size);
        new_block->size = current->size - size - sizeof(MemBlock);
        new_block->next = current->next;
        new_block->prev = current;
        new_block->used = FALSE;
        if (current->next != NULL) {
          current->next->prev = new_block;
        }
        current->next = new_block;
      }
      // Update the current block size and return the allocated memory
      current->size = size;
      current->used = TRUE;
      return (void*)(current + 1); // Skip the MemBlock header
    }
    previous = current;
    // Move to the next block
    current = current->next;
  }

  Con_Error("VM: Out of memory for block of %zu bytes", size);
  // No suitable block found
  return NULL;
}

// Function to free previously allocated memory
void
vm_freemem(PVM vm, void* ptr)
{
  if (ptr == vm->memory)
    return;

  //    Con_Error("free:: %llx", ptr);

  // Find the MemBlock header before the given pointer
  MemBlock* mem = ((MemBlock*)ptr) - 1;

  mem->used = FALSE;

  if (mem->prev && !mem->prev->used) {
    mem->prev->next = mem->next;
    if (mem->next) {
      mem->next->prev = mem->prev;
    }
    mem->prev->size += mem->size + sizeof(MemBlock);
    mem = mem->prev;
  }

  if (mem->next && !mem->next->used) {
    size_t next_size = mem->next->size;
    mem->next = mem->next->next;
    if (mem->next) {
      mem->next->prev = mem;
    }
    mem->size += next_size + sizeof(MemBlock);
  }
}

void*
vm_reallocmem(PVM vm, void* ptr, size_t size)
{
  void* mem = vm_allocmem(vm, size);
  if (ptr != vm->memory) {
    MemBlock* mem = ((MemBlock*)ptr) - 1;
    memcpy(mem, ptr, MIN(mem->size, size));
    vm_freemem(vm, ptr);
  }
  return mem;
}

void
test_mem(PVM vm)
{
  for (MemBlock *n = vm->head, *p = vm->head, *p2 = vm->head; n;
       p2 = p, p = n, n = n->next) {
    //        Con_Error("  %d", n->size);
  }

  //    Con_Error("\n");
}
