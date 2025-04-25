#include "dmg.h"

#include <stdckdint.h>

namespace DMG {
  void DMG::execute_block2(uint8_t opcode) {
    char src = opcode & 0b111;
    char aluOP = (opcode >> 3) & 0b111;
    bool halfCarry, carry = false;
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
}