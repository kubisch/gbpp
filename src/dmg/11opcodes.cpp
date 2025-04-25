#include <stdckdint.h>

#include "dmg.h"

namespace DMG {
  void DMG::execute_block3(uint8_t opcode) {
    switch (opcode & 0b111) {
      case 0b110:  // Operations on A
      {
        bool halfCarry, carry = false;
        unsigned char intermediate = 0;
        switch (opcode >> 3 & 0b111) {
          case 0b000:  // ADD A, n8
            halfCarry = (*A & 0xF) + mem[++PC.val] > 0xF;
            carry = ckd_add(A, *A, mem[PC.val]);
            *F = (*F & 0xF) | mk_flags(*A == 0, false, halfCarry, carry);

            break;

          case 0b001:  // ADC A, n8
            halfCarry = (*A & 0xF) + mem[++PC.val] + ((*F & CARRY) >> 4) > 0xF;
            carry = ckd_add(&intermediate, mem[PC.val], (*F & CARRY) >> 4);
            carry |= ckd_add(A, *A, intermediate);
            *F = (*F & 0xF) | mk_flags(*A == 0, false, halfCarry, carry);

            break;

          case 0b010:  // SUB A, n8
            halfCarry = (*A & 0xF) < mem[++PC.val];
            carry = ckd_sub(A, *A, mem[PC.val]);
            *F = (*F & 0xF) | mk_flags(*A == 0, true, halfCarry, carry);

            break;

          case 0b011:  // SBC A, n8
            halfCarry = (*A & 0xF) < mem[++PC.val] + 1;
            carry = ckd_add(&intermediate, mem[PC.val], 1);
            carry |= ckd_sub(A, *A, intermediate);
            *F = (*F & 0xF) | mk_flags(*A == 0, true, halfCarry, carry);

            break;
          
          case 0b100:  // AND A, n8
            *A &= mem[++PC.val];
            *F = mk_flags(*A == 0, false, true, false);

            break;

          case 0b101:  // XOR A, n8
            *A ^= mem[++PC.val];
            *F = mk_flags(*A == 0, false, false, false);

            break;

          case 0b110:  // OR A, n8
            *A |= mem[++PC.val];
            *F = mk_flags(*A == 0, false, false, false);

            break;
          
          case 0b111:  // CP A, n8
            intermediate = mem[++PC.val];
            *F = mk_flags(
              *A == intermediate,
              true,
              (*A & 0xF) < (intermediate & 0xF),
              *A < intermediate
            );

            break;
        }

        cycles += 8;
        PC.val++;

        break;
      }

      case 0b111:  // RST addr
        PC.val++;
        mem[--SP.val] = PC.hi;
        mem[--SP.val] = PC.lo;
        PC.hi = 0x08 * ((opcode >> 3) & 0x111);

        cycles += 16;
        break;
    }
  }
}