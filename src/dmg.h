#pragma once
#include <stdint.h>
#include <cstdio>

#include <array>

#define ZERO (1 << 7)
#define SUB (1 << 6)
#define HALF_CARRY (1 << 5)
#define CARRY (1 << 4)

namespace DMG {

  typedef uint8_t R8;

  union R16 {
    public:
      struct {
        public:
          R8 lo;
          R8 hi;
      };
      uint16_t val;
  };

  class DMG {
    public:
      void init();
      void execute();
      void load(unsigned char arr[], uint16_t size, uint16_t offset);

      signed char peak() { return mem[PC.val]; }
      void repr() {
        printf(
          "AF: %04X BC: %04X DE: %04X HL: %04X\n",
          AF.val,
          BC.val,
          DE.val,
          HL.val
        );
        printf("SP: %04X PC: %04X\n", SP.val, PC.val);
        printf(
          "Flags: %i%i%i%i\n",
          (*F & ZERO) >> 7,
          (*F & SUB) >> 6,
          (*F & HALF_CARRY) >> 5,
          (*F & CARRY) >> 4
        );
        printf("Cycles: %d\n\n", cycles);
      }

    private:
      R16 AF, BC, DE, HL = {.val = 0};
      R16 SP, PC = {.val = 0};

      unsigned int cycles = 0;

      R8 *A = &AF.hi;
      R8 *F = &AF.lo;
      R8 *B = &BC.hi;
      R8 *C = &BC.lo;
      R8 *D = &BC.hi;
      R8 *E = &BC.lo;
      R8 *H = &BC.hi;
      R8 *L = &BC.lo;

      std::array<uint8_t, 1 << 16> mem;

      void execute_block0(uint8_t opcode);
      void execute_block1(uint8_t opcode);
      void execute_block2(uint8_t opcode);

      uint8_t mk_flags(bool zero, bool sub, bool half_carry, bool carry);
      R8 *decode_r8(uint8_t index);
      R16 *decode_r16(uint8_t index);
      // char* decode_r16stk(char index);
      // char* decode_r16mem(char index);
  };
}