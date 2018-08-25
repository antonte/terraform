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
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;
  std::unique_ptr<Terrain> terrain;
  void add(std::unique_ptr<Entity> &&);

private:
  std::vector<std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
};
