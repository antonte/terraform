#include "asm.hpp"

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
  loop,
  chargeProgram, // address of charge program
  buildProgram, // address of build program
  energy,     // addess to energy
  lowEnergy,  // low energy threshold, if bot riches this level it switches to charge mode
  hiEnergy,   // hi energy threshold, if bot in charge mode, riching this level switches bot to work
              // mode
  matter,
  buildLevel, 
  stone_scan, // address to stone scan microprogram
  move,       // address to move port
};
const Program Program::Default = Program{
  // setting up the vars
  LOAD(R[energy], 0xffff),
  R[lowEnergy] |= 6,
  LOAD(R[hiEnergy], 100),
  LOAD(R[move], 0xfff6),
  LOAD(R[stone_scan], 0xfff9),
  LOAD(R[matter], 0xfffe),
  LOAD(R[buildLevel], 75),
  LOAD(R[chargeProgram], 34),
  LOAD(R[buildProgram], 39),

  // begin
  R[loop] = R[ip],

  R[tmp1] = *R[energy],     // check if we have enought energy
  R[tmp1] -= R[lowEnergy],
  R[tmp1] < R[chargeProgram],

  R[tmp1] = *R[matter], // check if we have enought matter
  R[tmp1] -= R[buildLevel],
  R[tmp1] > R[buildProgram],

  *R[move] = *R[stone_scan], // move to recomended direction
  R[ip] = R[loop],

  // charge program
  *R[move] ^= *R[move],
  R[tmp1] = *R[energy],
  R[tmp1] -= R[hiEnergy],
  R[tmp1] < R[chargeProgram],
  R[ip] = R[loop],

  // build program
  R[tmp1] ^= R[tmp1],
  R[tmp1] |= 6,
  *R[move] = R[tmp1],
  R[ip] = R[loop],
};
