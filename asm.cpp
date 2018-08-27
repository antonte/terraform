#include "asm.hpp"
#include <cstdlib>

struct RegPtr;

Program::Program(std::initializer_list<OpCode> l)
{
  for (int j = 0; j < 1024; ++j)
    ram[j] = rand() % 0x10000;
  auto i = std::begin(ram);
  for (auto j : l)
    *i++ = j.data;
}

Ram Program::data() const
{
  return ram;
}

enum {
  ip,
  tmp1,
  lowEnergy,  // low energy threshold, if bot riches this level it switches to charge mode
  hiEnergy,   // hi energy threshold, if bot in charge mode, riching this level switches bot to work
              // mode
  energy,     // addess to energy
  stone_scan, // address to stone scan microprogram
  move,       // address to move port
  chargeProgram, // address of charge program
  loop
};
const Program Program::Default = Program{
  // setting up the vars
  R[tmp1] |= 6,          // 1
  R[energy] |= 0x3f,     // 2
  R[energy] <<= R[tmp1], // 3
  R[energy] |= 0x3f,     // 4
  R[energy] <<= R[tmp1], // 5
  R[energy] |= 0x3f,     // 6 energy = 0xffff

  R[lowEnergy] |= 10, // 7 lowEnergy = 10

  R[tmp1] ^= R[tmp1],         // 8
  R[tmp1] |= 8,               // 9
  R[hiEnergy] ^= R[hiEnergy], // 10
  R[hiEnergy] |= 4,           // 11
  R[hiEnergy] <<= R[tmp1],    // 12 highEnergy = 1024

  R[move] = R[energy], // 13
  R[tmp1] ^= R[tmp1],  // 14
  R[tmp1] |= 0x9,      // 15
  R[move] -= R[tmp1],  // 16 move = 0xfff6

  R[stone_scan] = R[energy], // 17
  R[tmp1] ^= R[tmp1],        // 18
  R[tmp1] |= 0x6,            // 19
  R[stone_scan] -= R[tmp1],  // 20 stone_scan = 0xfff9

  // begin
  R[loop] = R[ip],             // 21
  R[chargeProgram] = R[ip],    // 22
  R[tmp1] ^= R[tmp1],          // 23
  R[tmp1] |= 8,                // 24
  R[chargeProgram] += R[tmp1], // 25
  R[tmp1] = *R[energy],        // 26
  R[tmp1] -= R[lowEnergy],     // 27
  R[tmp1] < R[chargeProgram],  // 28
  *R[move] = *R[stone_scan],   // 29 move to recomended direction
  R[ip] = R[loop],             // 30

  // charge program
  *R[move] ^= *R[move],       // 31
  R[tmp1] = *R[energy],       // 32
  R[tmp1] -= R[hiEnergy],     // 33
  R[tmp1] < R[chargeProgram], // 34
  R[ip] = R[loop]             // 35
};
