#pragma once
#include <memory>
#include <shade/var.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class BotClass;
class Library;
class StoneClass;
class Terrain;
class Entity;

namespace sdl
{
  class Texture;
}

class World
{
public:
  World(Library &);
  ~World();
  void draw(Var<glm::mat4> &mvp, int minX, int maxX, int minY, int maxY);
  void tick();
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;
  std::unique_ptr<Terrain> terrain;
  void add(std::unique_ptr<Entity> &&);
  float getO2Level() const;
  float getWaterLevel() const;
  float getTreeLevel() const;
  void move(Entity &, float x, float y);
  std::vector<Entity*> getAround(float x, float y) const;
  void remove(Entity &);

private:
  std::unordered_map<Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
  std::unordered_set<Entity *> active;
};
