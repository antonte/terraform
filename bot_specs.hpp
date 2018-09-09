#pragma once
#include "asm.hpp"

struct BotSpecs
{
  int lifeSpan = 20000;
  int batteryCapacity = 2000;
  int chargeRate = 400;
  int speed = 400;
  int buildTime = 1000;
  int takeTime = 100;
  int buildEnergy = 400;
  int o2PlantLifeSpan = 12000;
  int h2OPlantLifeSpan = 12000;
  int o2PlantProdRate = 100;
  int h2OPlantProdRate = 100;
  int64_t botPrice = 100000000;
  Ram ram = Program::Default.data();
};
