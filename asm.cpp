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

enum { ip, tmp1, energy, stone_scan, move, loop };
const Program Program::Default = Program{R[tmp1] |= 6,
                                         R[energy] |= 0x3f,
                                         R[energy] <<= R[tmp1],
                                         R[energy] |= 0x3f,
                                         R[energy] <<= R[tmp1],
                                         R[energy] |= 0x3f, // energy = 0xffff

                                         R[move] = R[energy],
                                         R[tmp1] ^= R[tmp1],
                                         R[tmp1] |= 0x9,
                                         R[move] -= R[tmp1], // move = 0xfff6

                                         R[stone_scan] = R[energy],
                                         R[tmp1] ^= R[tmp1],
                                         R[tmp1] |= 0x6,
                                         R[stone_scan] -= R[tmp1], // stone_scan = 0xfff9

                                         R[loop] = R[ip],
                                         *R[move] = *R[stone_scan], // move to recomended direction
                                         R[ip] = R[loop]};
