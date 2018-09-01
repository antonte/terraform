#include "bot.hpp"
#include "bot_class.hpp"
#include "h2o_plant.hpp"
#include "o2_plant.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "tree.hpp"
#include "world.hpp"
#include <SDL.h>
#include <algorithm>
#include <coeff/coefficient_registry.hpp>
#include <log/log.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(BotSpeed, 0.03f);
COEFF(BotRotationSpeed, 0.03f);
COEFF(BotChargeRate, 3);
COEFF(BuildEnergy, 300);
COEFF(SpawnDistance, 5);
COEFF(TTL, 1000.0f);
COEFF(TakeTime, 100.0f);
COEFF(BuildTime, 1000.0f);

int Bot::lastId = 0;

Bot::Bot(World &world,
         float x,
         float y,
         uint16_t aMaxEnergy,
         uint16_t aMaxMatter,
         int aMaxAge,
         const Ram &aRam)
  : Entity(world, x, y),
    id(lastId++),
    maxEnergy(aMaxEnergy),
    maxMatter(aMaxMatter),
    maxAge(aMaxAge),
    ram(aRam)
{
  for (auto &r: reg)
    r = 0;
  ++world.botsNum;
  // for (auto i = 0; i < RamSize; ++i)
  //   LOG(i, opCodeToString(ram[i]));
}

Bot::~Bot()
{
  --world->botsNum;
}

void Bot::draw()
{
  world->mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y))) *
        glm::rotate(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  world->botClass->energy = 1.0 * energy / maxEnergy;
  world->botClass->matter = 1.0 * matter / maxMatter;
  world->botShad->use();

  auto getBuildingObjMvp = [this]() {
    return glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y) + 10)) *
           glm::rotate(direction - 3.1415926f / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
           glm::translate(glm::vec3(SpawnDistance, SpawnDistance, 0.0f)) *
           glm::rotate(SDL_GetTicks() * 0.001f, glm::vec3(0.0f, 0.0f, 1.0f));
  };

  if (energy < BotChargeRate + 1)
  {
    auto &&anim = world->botClass->idleAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    return;
  }
  switch (move)
  {
  case Move::Stop:
  {
    auto &&anim = world->botClass->idleAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    break;
  }
  case Move::Frwd:
  {
    auto &&anim = world->botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Right:
  {
    auto &&anim = world->botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Bckwd:
  {
    auto &&anim = world->botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Left:
  {
    auto &&anim = world->botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Take:
  {
    auto &&anim = world->botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::BuildBot:
  {
    auto &&anim = world->botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();

    world->mvp = getBuildingObjMvp();
    world->buildShad->use();
    auto &&buildAnim = world->botClass->idleAnim;
    buildAnim[SDL_GetTicks() * 30 / 1000 % buildAnim.size()]->draw();
  }
  break;
  case Move::BuildO2:
  {
    auto &&anim = world->botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    world->mvp = getBuildingObjMvp();
    world->buildShad->use();
    world->o2PlantObj->draw();
  }
  break;
  case Move::BuildWater:
  {
    auto &&anim = world->botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    world->mvp = getBuildingObjMvp();
    world->buildShad->use();
    world->h2OPlantObj->draw();
  }
  break;
  case Move::PlantTree:
  {
    auto &&anim = world->botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    world->mvp = getBuildingObjMvp();
    world->buildShad->use();
    world->treeObj->draw();
  }
  break;
  }
}

static float sign(float val)
{
  if (val < 0)
    return -1.0f;
  else
    return 1.0f;
}

void Bot::tick()
{
  if (rand() % 40000 == 0)
    ram[rand() % RamSize] = rand() % 0x10000; // make bot code mutate
  ++age;
  if (age > maxAge /*&& id != 0*/)
    world->kill(*this);

  botCrowdMeter *= 0.9999f;
  o2CrowdMeter *= 0.9999f;
  h2oCrowdMeter *= 0.9999f;
  treeCrowdMeter *= 0.9999f;

  if (busyCount > 0)
  {
    --busyCount;
    return;
  }

  if (x < -Terrain::Width / 2.0f || x > Terrain::Width / 2.0f || y < -Terrain::Height / 2.0f ||
      y > Terrain::Height / 2.0f)
    direction = atan2(-sign(y) * abs(sin(direction)), -sign(x) * abs(cos(direction)));

  switch (move)
  {
  case Move::Stop:
    energy = std::min(energy + BotChargeRate, static_cast<int>(maxEnergy));
    break;
  case Move::Frwd:
    if (energy < 1)
      break;
    energy -= 1;
    world->move(*this, x + BotSpeed * cos(direction), y + BotSpeed * sin(direction));
    break;
  case Move::Right:
    if (energy < 1)
      break;
    energy -= 1;
    direction -= BotRotationSpeed;
    if (direction < -3.1415926f)
      direction += 2 * 3.1415926f;
    break;
  case Move::Bckwd:
    if (energy < 1)
      break;
    energy -= 1;
    world->move(*this, x - BotSpeed * cos(direction), y - BotSpeed * sin(direction));
    break;
  case Move::Left:
    if (energy < 1)
      break;
    energy -= 1;
    direction += BotRotationSpeed;
    if (direction > 3.1415926f)
      direction -= 2 * 3.1415926f;
    break;
  case Move::Take:
  {
    move = Move::Stop;
    if (energy < TakeEnergy)
      break;
    if (matter + Stone::Matter > maxMatter)
      break;
    Entity *clEnt = nullptr;
    float minDist;
    for (auto &&ent : world->getAround(x, y))
    {
      if (ent == this)
        continue;
      if (!dynamic_cast<Stone *>(ent))
        continue;
      auto dist = std::hypot(ent->getX() - x, ent->getY() - y);
      if (!clEnt || dist < minDist)
      {
        clEnt = ent;
        minDist = dist;
        continue;
      }
    }
    if (!clEnt)
      break;
    if (minDist >= 0.5f)
      break;
    energy -= TakeEnergy;
    matter += Stone::Matter;
    world->remove(*clEnt);
    break;
  }
  case Move::BuildBot:
    move = Move::Stop;
    if (energy < BuildEnergy)
      break;
    if (matter < Bot::Matter)
      break;
    energy -= BuildEnergy;
    matter -= Bot::Matter;
    world->add(std::make_unique<Bot>(*world,
                                     x + SpawnDistance * cos(direction),
                                     y + SpawnDistance * sin(direction),
                                     maxEnergy,
                                     maxMatter,
                                     maxAge,
                                     ram));
    break;
  case Move::BuildO2:
    move = Move::Stop;
    if (energy < BuildEnergy)
      break;
    if (matter < Bot::Matter)
      break;
    energy -= BuildEnergy;
    matter -= Bot::Matter;
    world->add(std::make_unique<O2Plant>(
      *world, 12000, x + SpawnDistance * cos(direction), y + SpawnDistance * sin(direction)));
    break;
  case Move::BuildWater:
    move = Move::Stop;
    if (energy < BuildEnergy)
      break;
    if (matter < Bot::Matter)
      break;
    energy -= BuildEnergy;
    matter -= Bot::Matter;
    world->add(std::make_unique<H2OPlant>(
      *world, 12000, x + SpawnDistance * cos(direction), y + SpawnDistance * sin(direction)));
    break;
  case Move::PlantTree:
    move = Move::Stop;
    if (energy < BuildEnergy)
      break;
    if (matter < Bot::Matter)
      break;
    energy -= BuildEnergy;
    matter -= Bot::Matter;
    world->add(std::make_unique<Tree>(
      *world, 12000, x + SpawnDistance * cos(direction), y + SpawnDistance * sin(direction)));
    break;
  }

  auto opCode = getRam(reg[0]++); // reg[0] is IP (Instruction Pointer) register
  // LOG(reg[0], opCodeToString(opCode), energy, reg[1], reg[7]);
  auto isLeftIndirect = (opCode & 0x0200) != 0;
  auto &leftReg = reg[(opCode >> 4) & 0x1f];
  auto value = reg[(opCode >> 10) & 0x1f];
  if ((opCode & 0x8000) != 0)
    value = getRam(value);
  switch (opCode % 16)
  {
  case OpCode::Nop: break;
  case OpCode::Mov:
    if (isLeftIndirect)
      setRam(leftReg, value);
    else
      leftReg = value;
    break;
  case OpCode::Load:
    if (isLeftIndirect)
      setRam(leftReg, (getRam(leftReg) << 6) | ((opCode >> 10) & 0x3f));
    else
      leftReg = (leftReg << 6) | ((opCode >> 10) & 0x3f);
    break;
  case OpCode::Add:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) + value);
    else
      leftReg += value;
    break;
  case OpCode::Sub:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) - value);
    else
      leftReg -= value;
    break;
  case OpCode::Mul:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) * value);
    else
      leftReg *= value;
    break;
  case OpCode::Div:
    if (value != 0)
    {
      if (isLeftIndirect)
        setRam(leftReg, getRam(leftReg) / value);
      else
        leftReg /= value;
    }
    else
    {
      if (isLeftIndirect)
        setRam(leftReg, 0x7fff);
      else
        leftReg = 0x7fff;
    }
    break;
  case OpCode::And:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) & value);
    else
      leftReg &= value;
    break;
  case OpCode::Or:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) | value);
    else
      leftReg |= value;
    break;
  case OpCode::Xor:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) ^ value);
    else
      leftReg ^= value;
    break;
  case OpCode::ShiftLeft:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) << value);
    else
      leftReg <<= value;
    break;
  case OpCode::ShiftRight:
    if (isLeftIndirect)
      setRam(leftReg, getRam(leftReg) >> value);
    else
      leftReg >>= value;
    break;
  case OpCode::Jeq:
    if (isLeftIndirect)
    {
      if (getRam(leftReg) == 0)
        reg[0] = value;
    }
    else
    {
      if (leftReg == 0)
        reg[0] = value;
    }
    break;
  case OpCode::Jne:
    if (isLeftIndirect)
    {
      if (getRam(leftReg) != 0)
        reg[0] = value;
    }
    else
    {
      if (leftReg != 0)
        reg[0] = value;
    }
    break;
  case OpCode::Jgt:
    if (isLeftIndirect)
    {
      if (getRam(leftReg) != 0 && (getRam(leftReg) & 0x8000) == 0)
        reg[0] = value;
    }
    else
    {
      if (leftReg != 0 && (leftReg & 0x8000) == 0)
        reg[0] = value;
    }
    break;
  case OpCode::Jlt:
    if (isLeftIndirect)
    {
      if (getRam(leftReg) != 0 && (getRam(leftReg) & 0x8000) != 0)
        reg[0] = value;
    }
    else
    {
      if (leftReg != 0 && (leftReg & 0x8000) != 0)
        reg[0] = value;
    }
    break;
  }
}

std::string Bot::opCodeToString(uint16_t opCode)
{
  std::ostringstream res;
  if (opCode % 16 != OpCode::Nop)
    res << ((opCode & 0x0200) != 0 ? "*" : "") << "R[" << ((opCode >> 4) & 0x1f) << "] ";

  switch (opCode % 16)
  {
  case OpCode::Nop: res << "nop"; break;
  case OpCode::Mov: res << "="; break;
  case OpCode::Load: res << "|="; break;
  case OpCode::Add: res << "+="; break;
  case OpCode::Sub: res << "-="; break;
  case OpCode::Mul: res << "*="; break;
  case OpCode::Div: res << "/="; break;
  case OpCode::And: res << "&="; break;
  case OpCode::Or: res << "|="; break;
  case OpCode::Xor: res << "^="; break;
  case OpCode::ShiftLeft: res << "<<="; break;
  case OpCode::ShiftRight: res << ">>="; break;
  case OpCode::Jeq: res << "=="; break;
  case OpCode::Jne: res << "!="; break;
  case OpCode::Jgt: res << ">"; break;
  case OpCode::Jlt: res << "<"; break;
  }
  if (opCode % 16 != OpCode::Nop && opCode % 16 != OpCode::Load)
    res << " " << ((opCode & 0x8000) != 0 ? "*" : "") << "R[" << ((opCode >> 10) & 0x1f) << "] ";
  else if (opCode % 16 == OpCode::Load)
    res << " " << ((opCode >> 10) & 0x3f);
  return res.str();
}

uint16_t Bot::getRam(uint16_t addr)
{
  if (addr < RamSize)
    return ram[addr];
  switch (static_cast<Addr>(addr))
  {
  case Addr::Move: return static_cast<uint16_t>(move);
  case Addr::StoneScan:
    {
      Entity* clEnt = nullptr;
      float minDist;
      for (auto &&ent: world->getAround(x, y))
      {
        if (ent == this)
          continue;
        if (dynamic_cast<Stone *>(ent))
        {
          auto dist = std::hypot(ent->getX() - x, ent->getY() - y);
          if (!clEnt || dist < minDist)
          {
            clEnt = ent;
            minDist = dist;
          }
        }
        else if (dynamic_cast<Bot *>(ent))
          ++botCrowdMeter;
        else if (dynamic_cast<O2Plant *>(ent))
          ++o2CrowdMeter;
        else if (dynamic_cast<H2OPlant *>(ent))
          ++h2oCrowdMeter;
        else if (dynamic_cast<Tree *>(ent))
          ++treeCrowdMeter;
      }
      if (!clEnt)
        return static_cast<uint16_t>(Move::Frwd);
      if (minDist < 0.5f)
        return static_cast<uint16_t>(Move::Take);
      auto ang = atan2(clEnt->getY() - y, clEnt->getX() - x) - direction;
      if (ang > 3.1415926f)
        ang -= 2.0f * 3.1415926f;
      if (ang < -3.1415926f)
        ang += 2.0f * 3.1415926f;
      if (abs(ang) < 3.1415926f / 8.0f)
        return static_cast<uint16_t>(Move::Frwd);
      if (ang < 0)
        return static_cast<uint16_t>(Move::Right);
      else
        return static_cast<uint16_t>(Move::Left);
    }
    return 0;
  case Addr::O2Level: return world->getO2Level() * 100;
  case Addr::WaterLevel: return world->getWaterLevel() * 100;
  case Addr::TreeLevel: return world->getTreeLevel() * 100;
  case Addr::Matter: return 100 * matter / maxMatter;
  case Addr::Energy: return 100 * energy / maxEnergy;
  }
  return 0;
}

void Bot::setRam(uint16_t addr, uint16_t value)
{
  if (addr < RamSize)
  {
    ram[addr] = value;
    return;
  }
  switch (static_cast<Addr>(addr))
  {
  case Addr::Move:
  {
    switch (static_cast<Move>(value))
    {
    case Move::Stop:
    case Move::Frwd:
    case Move::Right:
    case Move::Bckwd:
    case Move::Left: break;
    case Move::Take: busyCount = TakeTime; break;
    case Move::BuildBot:
      if (botCrowdMeter <= o2CrowdMeter && botCrowdMeter <= h2oCrowdMeter &&
          botCrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildBot);
      else if (o2CrowdMeter <= h2oCrowdMeter && o2CrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildO2);
      else if (h2oCrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildWater);
      else
        value = static_cast<int>(Move::PlantTree);
      busyCount = BuildTime;
      break;
    case Move::BuildO2:
    case Move::BuildWater:
    case Move::PlantTree: busyCount = BuildTime; break;
    }

    move = static_cast<Move>(value);
    break;
  }
  default: break;
  }
}

int Bot::getMatter() const
{
  return matter + Matter;
}
