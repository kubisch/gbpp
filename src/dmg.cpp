#include "dmg.h"

#include <stdckdint.h>

#include <cstdint>

// TODO:
// - Implement HALT
// - Implement STOP

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
      execute_block0(opcode);
    } else if (block == 0b01) {
      execute_block1(opcode);
    } else if (block == 0b10) {
      execute_block2(opcode);
    } else if (block == 0b11) {
      // execute_block3(opcode);
    }
  }

  void DMG::execute_block0(uint8_t opcode) {
    bool halfCarry, carry;
    char dest;
    uint8_t data;
    uint16_t bigData;

    switch (opcode & 0b111) {
      case 0b000:
        switch (opcode >> 3 & 0b111) {
          case 0b000:  // NOP
            cycles += 4;
            PC.val++;
            break;

          case 0b001:  // LD [a16], SP
            bigData = mem[++PC.val];
            bigData |= mem[++PC.val] << 8;
            mem[bigData] = SP.lo;
            mem[bigData + 1] = SP.hi;

            cycles += 20;
            PC.val++;
            break;

          case 0b010:  // STOP
            // TODO: implement STOP
            cycles += 4;
            PC.val++;
            PC.val++;
            break;

          case 0b011:  // JR (s8)
            PC.val += (int8_t)mem[++PC.val];
            cycles += 12;
            break;

          // Opcodes 00 1XX 000
          // JR cond (s8)
          default:
            if (decode_condition(opcode >> 3 & 0b11)) {
              PC.val += (int8_t)mem[++PC.val];
              cycles += 12;
            } else {
              PC.val++;
              cycles += 8;
            }
            break;
        }
        break;

      case 0b001:
        dest = (opcode >> 4) & 0b11;

        if (opcode & 010) {  // ADD HL
          halfCarry =
            (HL.val & 0x0FFF) + ((*decode_r16(dest)).val & 0x0FFF) > 0x0FFF;
          carry = ckd_add(&HL.val, HL.val, (*decode_r16(dest)).val);

          *F = (*F & 0b10001111) | mk_flags(false, false, halfCarry, carry);

          cycles += 8;
          PC.val++;
        } else {  // LD [r16], n16
          bigData |= mem[++PC.val];
          bigData |= mem[++PC.val] << 8;
          (*decode_r16(dest)).val = bigData;

          cycles += 12;
          PC.val++;
        }
        
        break;

      case 0b010:
        dest = (opcode >> 4) & 0b11;

        if (opcode & 010) {  // A <- [r16]
          *A = mem[(*decode_r16(dest)).val];
        } else {  // [r16] <- A
          mem[(*decode_r16(dest)).val] = *A;
        }

        cycles += 8;
        PC.val++;
        break;

      case 0b011:  // INC/DEC R16
        dest = (opcode >> 4) & 0b11;
        if (opcode & 010) {  // DEC (mask out 00001000, which is 010 in octal)
          (*decode_r16(dest)).val--;
        } else {  // INC
          (*decode_r16(dest)).val++;
        }

        cycles += 8;
        PC.val++;
        break;

      case 0b100:  // INC R8
        dest = (opcode >> 3) & 0b111;
        halfCarry = (*decode_r8(dest) & 0xF) == 0xF;

        (*decode_r8(dest))++;
        *F = (*F & 0b00011111)
          | mk_flags(*decode_r8(dest) == 0, false, halfCarry, false);

        if (dest == 6) {
          cycles += 12;
        } else {
          cycles += 4;
        }

        PC.val++;
        break;

      case 0b101:  // DEC R8
        dest = (opcode >> 3) & 0b111;
        halfCarry = (*decode_r8(dest) & 0xF) == 0;

        (*decode_r8(dest))--;
        *F = (*F & 0b00011111)
          | mk_flags(*decode_r8(dest) == 0, true, halfCarry, false);

        if (dest == 6) {
          cycles += 12;
        } else {
          cycles += 4;
        }

        PC.val++;
        break;

      case 0b110:  // LD R8, n8
        dest = (opcode >> 3) & 0b111;
        data = mem[++PC.val];

        *decode_r8(dest) = data;

        if (dest == 6) {
          cycles += 12;
        } else {
          cycles += 4;
        }

        PC.val++;
        break;

      case 0b111:  // Misc. bit operations
        uint8_t adjustment = 0;
        switch (opcode >> 3 & 0b111) {
          case 0b000:  // RLCA
            carry = (bool)(*A >> 7 & 0b1);
            *A = (*A << 1) | carry;

            *F = (*F & 0xF) | mk_flags(false, false, false, carry);
            break;

          case 0b001:  // RRCA
            carry = (bool)(*A & 0b1);
            *A = (*A >> 1) | (carry << 7);

            *F = (*F & 0xF) | mk_flags(false, false, false, carry);
            break;

          case 0b010:  // RLA
            carry = (bool)(*A >> 7 & 0b1);
            *A = (*A << 1) | (*F & CARRY) >> 4;

            *F = (*F & 0xF) | mk_flags(false, false, false, carry);
            break;

          case 0b011:  // RRA
            carry = (bool)(*A & 0b1);
            *A = (*A >> 1) | (*F & CARRY) << 3;

            *F = (*F & 0xF) | mk_flags(false, false, false, carry);
            break;

          case 0b100:  // DAA
            if ((*F & SUB) >> 6) {
              adjustment =
                (*F & CARRY ? 0x60 : 0x00) | (*F & HALF_CARRY ? 0x06 : 0x00);
              carry = ckd_sub(A, *A, adjustment);

              *F = (*F & 0xF) | mk_flags(*A == 0, true, false, carry);
            } else {
              adjustment = (((*A > 0x99) || (*F & CARRY)) ? 0x60 : 0x00)
                | (((*A & 0xF) > 0x9 || (*F & HALF_CARRY)) ? 0x06 : 0x00);
              carry = ckd_add(A, *A, adjustment);

              *F = (*F & 0x01001111) | mk_flags(*A == 0, false, false, carry);
            }
            break;

          case 0b101:  // CPL
            *A = ~*A;
            *F = (*F & 0b1001111) | mk_flags(false, true, true, false);
            break;

          case 0b110:  // SCF
            *F = (*F & 0b10001111) | CARRY;
            break;

          case 0b111:  // CCF
            *F = (*F & 0b10001111) | ~(*F & CARRY);
            break;
        }

        cycles += 4;
        PC.val++;
        break;
    }
  }

  void DMG::execute_block1(uint8_t opcode) {
    bool isHalt = opcode == 0x76;

    // TODO: implement HALT

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

  bool DMG::decode_condition(uint8_t index) {
    switch (index) {
      case 0b00: // NZ
        return !(bool)(*F & ZERO);

      case 0b01: // Z
        return (bool)(*F & ZERO);

      case 0b10: // NC
        return !(bool)(*F & CARRY);

      case 0b11: // C
        return (bool)(*F & CARRY);
    }
  }
}
