#include "bot.hpp"

#include "bot_class.hpp"
#include "h2o_plant.hpp"
#include "o2_plant.hpp"
#include "pi.hpp"
#include "rend.hpp"
#include "stone.hpp"
#include "terrain.hpp"
#include "tree.hpp"
#include "world.hpp"
#include <coeff/coefficient_registry.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(BotRotationSpeed, 0.03f);
COEFF(SpawnDistance, 5);

int Bot::lastId = 0;

Bot::Bot(World &world, float x, float y, const BotSpecs &aSpecs)
  : Entity(world, x, y), specs(aSpecs), id(lastId++)
{
  for (auto &r: reg)
    r = 0;
  ++world.botsNum;
}

Bot::~Bot()
{
  --world->botsNum;
}

void Bot::draw(Rend &rend)
{
  rend.mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y))) *
        glm::rotate(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  rend.botClass->energy = 1.0 * energy / specs.batteryCapacity;
  rend.botClass->matter = 1.0 * matter / MaxMatter;
  rend.botShad->use();

  auto getBuildingObjMvp = [this]() {
    return glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y) + 10)) *
           glm::rotate(direction - PI / 4.0f, glm::vec3(0.0f, 0.0f, 1.0f)) *
           glm::translate(glm::vec3(SpawnDistance, SpawnDistance, 0.0f)) *
           glm::rotate(SDL_GetTicks() * 0.001f, glm::vec3(0.0f, 0.0f, 1.0f));
  };

  if (energy < specs.chargeRate / 100 + 1)
  {
    auto &&anim = rend.botClass->idleAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    return;
  }
  switch (move)
  {
  case Move::Stop:
  {
    auto &&anim = rend.botClass->idleAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    break;
  }
  case Move::Frwd:
  {
    auto &&anim = rend.botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Right:
  {
    auto &&anim = rend.botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Bckwd:
  {
    auto &&anim = rend.botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Left:
  {
    auto &&anim = rend.botClass->walkAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::Take:
  {
    auto &&anim = rend.botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
  }
  break;
  case Move::BuildBot:
  {
    auto &&anim = rend.botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();

    rend.mvp = getBuildingObjMvp();
    rend.buildShad->use();
    auto &&buildAnim = rend.botClass->idleAnim;
    buildAnim[SDL_GetTicks() * 30 / 1000 % buildAnim.size()]->draw();
  }
  break;
  case Move::BuildO2Plant:
  {
    auto &&anim = rend.botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    rend.mvp = getBuildingObjMvp();
    rend.buildShad->use();
    rend.o2PlantObj->draw();
  }
  break;
  case Move::BuildH2OPlant:
  {
    auto &&anim = rend.botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    rend.mvp = getBuildingObjMvp();
    rend.buildShad->use();
    rend.h2OPlantObj->draw();
  }
  break;
  case Move::PlantTree:
  {
    auto &&anim = rend.botClass->busyAnim;
    anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
    rend.mvp = getBuildingObjMvp();
    rend.buildShad->use();
    rend.treeObj->draw();
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
    specs.ram[rand() % RamSize] = rand() % 0x10000; // make bot code mutate
  ++age;
  if (age > specs.lifeSpan)
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
    // TODO fix rounding
    energy = std::min(energy + specs.chargeRate / 100, static_cast<int>(specs.batteryCapacity));
    break;
  case Move::Frwd:
    if (energy < 1)
      break;
    energy -= 1;
    world->move(*this,
                x + specs.speed / 10000.0f * cos(direction),
                y + specs.speed / 10000.0f * sin(direction));
    break;
  case Move::Right:
    if (energy < 1)
      break;
    energy -= 1;
    direction -= BotRotationSpeed;
    if (direction < -PI)
      direction += 2 * PI;
    break;
  case Move::Bckwd:
    if (energy < 1)
      break;
    energy -= 1;
    world->move(*this,
                x - specs.speed / 10000.0f * cos(direction),
                y - specs.speed / 10000.0f * sin(direction));
    break;
  case Move::Left:
    if (energy < 1)
      break;
    energy -= 1;
    direction += BotRotationSpeed;
    if (direction > PI)
      direction -= 2 * PI;
    break;
  case Move::Take:
  {
    move = Move::Stop;
    break;
  }
  case Move::BuildBot:
    move = Move::Stop;
    if (energy < specs.buildEnergy)
    {
      energy = 0;
      break;
    }
    if (matter < Bot::Matter)
      break;
    energy -= specs.buildEnergy;
    matter -= Bot::Matter;
    world->add<Bot>(x + SpawnDistance * cos(direction), y + SpawnDistance * sin(direction), specs);
    break;
  case Move::BuildO2Plant:
    move = Move::Stop;
    if (energy < specs.buildEnergy)
    {
      energy = 0;
      break;
    }
    if (matter < Bot::Matter)
      break;
    energy -= specs.buildEnergy;
    matter -= Bot::Matter;
    world->add<O2Plant>(specs.o2PlantLifeSpan,
                        specs.o2PlantProdRate,
                        x + SpawnDistance * cos(direction),
                        y + SpawnDistance * sin(direction));
    break;
  case Move::BuildH2OPlant:
    move = Move::Stop;
    if (energy < specs.buildEnergy)
    {
      energy = 0;
      break;
    }
    if (matter < Bot::Matter)
      break;
    energy -= specs.buildEnergy;
    matter -= Bot::Matter;
    world->add<H2OPlant>(specs.h2OPlantLifeSpan,
                         specs.h2OPlantProdRate,
                         x + SpawnDistance * cos(direction),
                         y + SpawnDistance * sin(direction));
    break;
  case Move::PlantTree:
    move = Move::Stop;
    if (energy < specs.buildEnergy)
    {
      energy = 0;
      break;
    }
    if (matter < Bot::Matter)
      break;
    energy -= specs.buildEnergy;
    matter -= Bot::Matter;
    world->add<Tree>(x + SpawnDistance * cos(direction), y + SpawnDistance * sin(direction));
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
    return specs.ram[addr];
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
      if (ang > PI)
        ang -= 2.0f * PI;
      if (ang < -PI)
        ang += 2.0f * PI;
      if (abs(ang) < PI / 8.0f)
        return static_cast<uint16_t>(Move::Frwd);
      if (ang < 0)
        return static_cast<uint16_t>(Move::Right);
      else
        return static_cast<uint16_t>(Move::Left);
    }
    return 0;
  case Addr::O2Level: return world->getO2Level() * 100;
  case Addr::H2OLevel: return world->getH2OLevel() * 100;
  case Addr::TreeLevel: return world->getTreeLevel() * 100;
  case Addr::Matter: return 100 * matter / MaxMatter;
  case Addr::Energy: return 100 * energy / specs.batteryCapacity;
  }
  return 0;
}

void Bot::setRam(uint16_t addr, uint16_t value)
{
  if (addr < RamSize)
  {
    specs.ram[addr] = value;
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
    case Move::Take:
    {
      busyCount = specs.takeTime;
      if (energy < TakeEnergy)
        break;
      if (matter + Stone::Matter > MaxMatter)
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
      if (botCrowdMeter <= o2CrowdMeter && botCrowdMeter <= h2oCrowdMeter &&
          botCrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildBot);
      else if (o2CrowdMeter <= h2oCrowdMeter && o2CrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildO2Plant);
      else if (h2oCrowdMeter <= treeCrowdMeter)
        value = static_cast<int>(Move::BuildH2OPlant);
      else
        value = static_cast<int>(Move::PlantTree);
      busyCount = specs.buildTime;
      break;
    case Move::BuildO2Plant:
    case Move::BuildH2OPlant:
    case Move::PlantTree: busyCount = specs.buildTime; break;
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
