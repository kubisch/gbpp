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
      case 0b00:  // NZ
        return !(bool)(*F & ZERO);

      case 0b01:  // Z
        return (bool)(*F & ZERO);

      case 0b10:  // NC
        return !(bool)(*F & CARRY);

      case 0b11:  // C
        return (bool)(*F & CARRY);

      default:
        return false;
    }
  }
}
