#include "dmg.h"

#include <stdckdint.h>

namespace DMG {
  void DMG::execute_block0(uint8_t opcode) {
    bool halfCarry, carry = false;
    char dest = 0;
    uint8_t data = 0;
    uint16_t bigData = 0;

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
          cycles += 8;
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
}