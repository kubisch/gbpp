#include <cassert>

#include "dmg.h"

int main() {
  DMG::DMG dmg;
  unsigned char test_program[] = {
    0x60,  // LD H, B        // NOP
    0x68,  // LD L, B        // NOP
    0x7E,  // LD A, [HL]     // A = 0x60
    0x86,  // ADD A, [HL]    // A = 0x60
    0x77,  // LD [HL], A     // mem[0] = 0x60
    0x47,  // LD B, A        // B = 0x60
    0x4F,  // LD C, A        // C = 0x60
    0xB0,  // OR B           // A = 0x60 | 0x60 = 0x60
    0xB9   // CP C           // Zero flag should be set
  };

  dmg.init();
  dmg.load(test_program, sizeof(test_program), 0);

  while (dmg.peak() != 0) {
    dmg.execute();
    dmg.repr();
  }

  return 0;
}