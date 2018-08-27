#pragma once
#include <memory>
#include <shade/var.hpp>
#include <vector>
#include <unordered_set>
#include <unordered_map>

class BotClass;
class Entity;
class Library;
class ShaderProgram;
class StoneClass;
class Terrain;

namespace sdl
{
  class Texture;
}

class World
{
public:
  World(Library &);
  ~World();
  void draw(float camX, float camY, float camZ);
  void tick();
  void add(std::unique_ptr<Entity> &&);
  float getO2Level() const;
  float getWaterLevel() const;
  float getTreeLevel() const;
  void move(Entity &, float x, float y);
  std::vector<Entity*> getAround(float x, float y) const;
  void remove(Entity &);

  static const int Width = 720 * 720 / 1280;
  static const int Height = 720;
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;
  std::unique_ptr<Terrain> terrain;
  Var<glm::mat4> mvp;
private:
  Var<glm::mat4> proj;
  Var<glm::mat4> view;
public:
  std::unique_ptr<ShaderProgram> shad;
  std::unique_ptr<ShaderProgram> botShad;

private:
  std::unordered_map<Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
  std::unordered_set<Entity *> active;
};
