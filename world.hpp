#pragma once
#include "sched.hpp"
#include <memory>
#include <shade/var.hpp>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class BotClass;
class Entity;
class ActiveEntity;
class Library;
class ShaderProgram;
class StoneClass;
class Terrain;
class Obj;
class ArrayBuffer;

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
  Entity *add(std::unique_ptr<Entity> &&);
  float getO2Level() const;
  float getWaterLevel() const;
  float getTreeLevel() const;
  void move(Entity &, float x, float y);
  std::vector<Entity *> getAround(float x, float y) const;
  void remove(Entity &);
  void kill(Entity &);

  static const int ScreenWidth = 720;
  static const int ScreenHeight = 720;
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;
  std::unique_ptr<Terrain> terrain;
  Var<glm::mat4> mvp;
  Var<float> o2Level;
  Var<float> h2OLevel;
  Var<float> time;
  std::unique_ptr<Obj> o2PlantObj;
  std::unique_ptr<Obj> h2OPlantObj;
  std::unique_ptr<Obj> treeObj;
  Sched sched;

private:
  Var<glm::mat4> proj;
  Var<glm::mat4> view;

public:
  std::unique_ptr<ShaderProgram> shad;
  std::unique_ptr<ShaderProgram> botShad;
  std::unique_ptr<ShaderProgram> terrainShad;
  std::unique_ptr<ShaderProgram> buildShad;
  int botsNum = 0;
  int o2PlantsNum = 0;
  int h2OPlantsNum = 0;
  int treesNum = 0;

private:
  std::unordered_map<Entity *, std::unique_ptr<Entity>> entities;
  std::unordered_map<uint32_t, std::unordered_set<Entity *>> grid;
  std::unordered_set<ActiveEntity *> active;
  std::unique_ptr<ArrayBuffer> waterMesh;
  std::unique_ptr<ShaderProgram> waterShad;
};
