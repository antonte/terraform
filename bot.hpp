#pragma once
#include "entity.hpp"
#include "active_entity.hpp"
#include "asm.hpp"

class Bot : public Entity, public ActiveEntity
{
public:
  Bot(World &,
      float x,
      float y,
      uint16_t maxEnergy,
      uint16_t maxMatter,
      int maxAge,
      const Ram &ram = Program::Default.data());
  ~Bot();
  void draw() override;
  void tick() override;
  static const int Matter = 100;
  int getMatter() const override;
private:
  static const int TakeEnergy = 10;
  enum class Addr {
    Move = 0xfff6,       // writing to this address makes bot to do something:
                         // 0 - stop
                         // 1 - move forward
                         // 2 - turn right
                         // 3 - move backward
                         // 4 - turn left
                         // 5 - take the stone
                         // 6 - build another bot
                         // 7 - build O2 generator
                         // 8 - build water generator
                         // 9 - plant the tree
    StoneScan = 0xfff9,  // reading this address will return recommended command to reach the
                         // closest stone
    O2Level = 0xfffa,    // level of O2 on the planet
    WaterLevel = 0xfffb, // amount of water on the planet
    TreeLevel = 0xfffc,  // amount of trees on the planet
    Matter = 0xfffe,     // reading from this address will return amount of matter the cell has
    Energy = 0xffff,     // reading from this address will return amount of energy the cell has
  };

  static int lastId;
  int id;
  uint16_t energy = 250;
  uint16_t matter = 0;
  int age = 0;
  uint16_t maxEnergy;
  uint16_t maxMatter;
  int maxAge;
  std::array<uint16_t, 32> reg; // reg[0] - ip
  Ram ram;
  uint16_t getRam(uint16_t addr);
  void setRam(uint16_t addr, uint16_t value);
  enum class Move {
    Stop = 0,
    Frwd = 1,
    Right = 2,
    Bckwd = 3,
    Left = 4,
    Take = 5,
    BuildBot = 6,
    BuildO2 = 7,
    BuildWater = 8,
    PlantTree = 9
  };
  Move move = Move::Stop;
  static std::string opCodeToString(uint16_t opCode);
  int busyCount = 0;
  float botCrowdMeter = 0.0f;
  float o2CrowdMeter = 100.0f;
  float h2oCrowdMeter = 100.0f;
  float treeCrowdMeter = 100.0f;
};

/*
  - Just standing will add 1-point of energy every tick
  - Moving will take away 2-points of energy every tick
  - Building will take away 400-points of energy and 100-points of matter and also will require
  50-ticks
  - Every stone will give 10-points of matter

  op code
  [][][][][][][][][][][][][][][][]
  [      ]-command
          []-is indirect
            [        ]-
                       0 ip register
                       1-31 register
                      []-is indirect
                        [        ]-
                                   0 ip register
                                   1-31 register
*/
