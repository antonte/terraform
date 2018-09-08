#pragma once
#include "sched.hpp"
#include <cstdint>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class ActiveEntity;
class Entity;
class Library;
class Rend;
class Terrain;

class World
{
public:
  World(Library &);
  ~World();

  void draw(Rend &, float camX, float camY, float camZ);
  void tick();
  float getO2Level() const;
  float getH2OLevel() const;
  float getTreeLevel() const;
  void move(Entity &, float x, float y);
  std::vector<Entity *> getAround(float x, float y, float rad = -1.0f) const;
  void remove(Entity &);
  void kill(Entity &);
  int getNow() const;
  bool isAlive(const Entity &) const;
  int64_t getIncome() const;

  template <typename T, typename... Args>
  Entity *add(Args &&... args)
  {
    return internalAdd(std::make_unique<T>(*this, std::forward<Args>(args)...));
  }

  Sched sched;
  std::unique_ptr<Terrain> terrain;

public:
  int botsNum = 0;
  int o2Rate = 0;
  int h2ORate = 0;
  int treesNum = 0;
  int64_t money = 1000000000;

private:
  Entity *internalAdd(std::unique_ptr<Entity> &&ent);
  int64_t realO2Level = 0;
  int64_t realH2OLevel = 0;
  std::unordered_map<const Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
  std::unordered_set<ActiveEntity *> active;
  int now = 0;
};
