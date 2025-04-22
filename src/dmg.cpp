#include "dmg.h"

#include <stdckdint.h>

#include <cstdint>

namespace DMG {
  void DMG::init() {
    AF.val = 0;
    BC.val = 0;
    DE.val = 0;
    HL.val = 0;

    SP.val = 0;
    PC.val = 0;

    mem.fill(0);
  }

  void DMG::load(uint8_t arr[], uint16_t size, uint16_t offset) {
    for (char i = 0; i < size; i++) {
      mem[offset + i] = arr[i];
    }
  }

  void DMG::execute() {
    uint8_t opcode = mem[PC.val];

    uint8_t block = opcode >> 6;

    if (block == 0b00) {
      // execute_block0(opcode);
    } else if (block == 0b01) {
      execute_block1(opcode);
    } else if (block == 0b10) {
      execute_block2(opcode);
    } else if (block == 0b11) {
      // execute_block3(opcode);
    }
  }

  void DMG::execute_block1(uint8_t opcode) {
    bool isHalt = opcode == 0x76;

    if (!isHalt) {
      char dest = (opcode >> 3) & 0b111;
      char src = opcode & 0b111;

      *decode_r8(dest) = *decode_r8(src);

      if ((dest == 6) or (src == 6)) {
        cycles += 8;
      } else {
        cycles += 4;
      }

      PC.val++;

      return;
    }
  }

  void DMG::execute_block2(uint8_t opcode) {
    char src = opcode & 0b111;
    char aluOP = (opcode >> 3) & 0b111;
    bool halfCarry, carry;
    unsigned char intermediate = 0;

    switch (aluOP) {
      case 0b000:  // ADD A,
        halfCarry = (*A & 0xF) + (*decode_r8(src) & 0xF) > 0xF;
        carry = ckd_add(A, *A, *decode_r8(src));

        *F = (*F & 0xF) | mk_flags(*A == 0, false, halfCarry, carry);
        break;

      case 0b001:  // ADC A,
        halfCarry =
          (*A & 0xF) + (*decode_r8(src) & 0xF) + ((*F & CARRY) >> 4) > 0xF;
        carry = ckd_add(&intermediate, *decode_r8(src), (*F & CARRY) >> 4);
        carry |= ckd_add(A, *A, intermediate);

        *F = (*F & 0xF) | mk_flags(*A == 0, false, halfCarry, carry);
        break;

      case 0b010:  // SUB A,
        halfCarry = (*A & 0xF) < (*decode_r8(src) & 0xF);
        carry = ckd_sub(A, *A, *decode_r8(src));

        *F = (*F & 0xF) | mk_flags(*A == 0, true, halfCarry, carry);
        break;

      case 0b011:  // SBC A,
        halfCarry = (*A & 0xF) < (*decode_r8(src) & 0xF) + 1;
        carry = ckd_add(&intermediate, *decode_r8(src), 1);
        carry |= ckd_sub(A, *A, intermediate);

        *F = (*F & 0xF) | mk_flags(*A == 0, true, halfCarry, carry);
        break;

      case 0b100:  // AND A,
        *A &= *decode_r8(src);

        *F = mk_flags(*A == 0, false, true, false);
        break;

      case 0b101:  // XOR A,
        *A ^= *decode_r8(src);

        *F = mk_flags(*A == 0, false, false, false);
        break;

      case 0b110:  // OR A,
        *A |= *decode_r8(src);

        *F = mk_flags(*A == 0, false, false, false);
        break;

      case 0b111:  // CP A,
        intermediate = *decode_r8(src);

        *F = mk_flags(
          *A == intermediate,
          true,
          (*A & 0xF) < (intermediate & 0xF),
          *A < intermediate
        );
        break;
    }

    if (src == 6) {
      cycles += 8;
    } else {
      cycles += 4;
    }

    PC.val++;

    return;
  }

  uint8_t DMG::mk_flags(bool zero, bool sub, bool half_carry, bool carry) {
    return (zero ? ZERO : 0) | (sub ? SUB : 0) | (half_carry ? HALF_CARRY : 0)
      | (carry ? CARRY : 0);
  }

  R8* DMG::decode_r8(uint8_t index) {
    switch (index) {
      case 0b000:
        return B;
      case 0b001:
        return C;
      case 0b010:
        return D;
      case 0b011:
        return E;
      case 0b100:
        return H;
      case 0b101:
        return L;
      case 0b110:
        return (uint8_t*)(mem.data() + HL.val);
      case 0b111:
        return A;
      default:
        return nullptr;
    }
  }

  R16* DMG::decode_r16(uint8_t index) {
    switch (index) {
      case 0b00:
        return &BC;
      case 0b01:
        return &DE;
      case 0b10:
        return &HL;
      case 0b11:
        return &SP;
      default:
        return nullptr;
    }
  }
}