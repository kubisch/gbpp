#include "dmg.h"

#include <stdckdint.h>

namespace DMG {
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
}