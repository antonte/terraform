#include "world.hpp"

#include "active_entity.hpp"
#include "bot.hpp"
#include "bot_class.hpp"
#include "entity.hpp"
#include "stone.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include <coeff/coefficient_registry.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(O2Level, 0.01f);
COEFF(H2OLevel, 0.01f);

static std::vector<glm::vec3> getWaterMesh()
{
  std::vector<glm::vec3> res;
  res.push_back(glm::vec3(-1.0f * Terrain::Width, -1.0f * Terrain::Height, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width, 1.0f * Terrain::Height, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width, -1.0f * Terrain::Height, 0.0f));

  res.push_back(glm::vec3(-1.0f * Terrain::Width, -1.0f * Terrain::Height, 0.0f));
  res.push_back(glm::vec3(-1.0f * Terrain::Width, 1.0f * Terrain::Height, 0.0f));
  res.push_back(glm::vec3(1.0f * Terrain::Width, 1.0f * Terrain::Height, 0.0f));
  return res;
}

World::World(Library &lib)
  : botClass(std::make_unique<BotClass>(lib)),
    stoneClass(std::make_unique<StoneClass>(lib)),
    terrain(std::make_unique<Terrain>(lib)),
    mvp("mvp"),
    o2Level("o2Level"),
    h2OLevel("h2OLevel"),
    time("time"),
    o2PlantObj(std::make_unique<Obj>(lib, "o2_plant")),
    h2OPlantObj(std::make_unique<Obj>(lib, "h2o_plant")),
    treeObj(std::make_unique<Obj>(lib, "tree")),
    proj("proj",
         glm::perspective(glm::radians(45.0f), 1.0f * ScreenWidth / ScreenHeight, 0.1f, 1000.0f)),
    view("view"),
    shad(std::make_unique<ShaderProgram>("shad", "shad", mvp, proj, view)),
    botShad(std::make_unique<ShaderProgram>("bot",
                                            "bot",
                                            mvp,
                                            proj,
                                            view,
                                            botClass->energy,
                                            botClass->matter)),
    terrainShad(
      std::make_unique<ShaderProgram>("terrain", "terrain", mvp, proj, view, o2Level, h2OLevel)),
    waterMesh(std::make_unique<ArrayBuffer>(getWaterMesh(), 0)),
    waterShad(std::make_unique<ShaderProgram>("water", "water", mvp, proj, view, h2OLevel, time))
{
}

World::~World() {}

static int getGridIdx(int x, int y)
{
  return (x + Terrain::Width / 2) / 10 + ((y + Terrain::Height / 2) / 10) * (Terrain::Width / 10);
}

COEFF(MapXK, 0.1f);
COEFF(MapYK, 0.1f);
COEFF(MapYBalance, 50);

void World::draw(float camX, float camY, float camZ)
{
  int minX = camX - camZ * MapXK;
  int maxX = camX + camZ * MapXK;
  int minY = camY - camZ * MapYK * MapYBalance / 100;
  int maxY = camY + camZ * MapYK;

  view = glm::lookAt(glm::vec3(camX, camY - camZ, camZ),
                     glm::vec3(camX, camY, 0.0f), // and looks at the origin
                     glm::vec3(0, 0, 1));
  terrain->draw(*this, minX, maxX, minY, maxY);
  for (auto y = minY - 10; y < maxY + 10; y += 10)
    for (auto x = minX - 10; x < maxX + 10; x += 10)
    {
      auto it = grid.find(getGridIdx(x, y));
      if (it == std::end(grid))
        continue;
      for (auto &&ent : it->second)
        ent->draw();
    }
  // draw water
  mvp = glm::translate(glm::vec3(0.0f, 0.0f, (getWaterLevel() * 10.0f - 15.0f)));
  time = SDL_GetTicks();
  waterShad->use();
  waterMesh->activate();
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

Entity *World::add(std::unique_ptr<Entity> &&ent)
{
  auto entPtr = ent.get();
  grid[getGridIdx(entPtr->getX(), entPtr->getY())].insert(entPtr);
  if (auto e = dynamic_cast<ActiveEntity *>(entPtr))
    active.insert(e);
  return entities.emplace(entPtr, std::move(ent)).first->second.get();
}

float World::getO2Level() const
{
  return O2Level;
}

float World::getWaterLevel() const
{
  return H2OLevel;
}

float World::getTreeLevel() const
{
  // TODO
  return 0.0f;
}

void World::tick()
{
  for (auto &&ent : active)
    ent->tick();

  for (auto &&ent : killArray)
    remove(*ent);
  killArray.clear();
}

void World::move(Entity &ent, float x, float y)
{
  auto idx1 = getGridIdx(ent.getX(), ent.getY());
  ent.setXY(x, y);
  auto idx2 = getGridIdx(ent.getX(), ent.getY());
  if (idx1 != idx2)
  {
    grid[idx1].erase(&ent);
    grid[idx2].insert(&ent);
  }
}

COEFF(LookupRadios, 5);

std::vector<Entity *> World::getAround(float xx, float yy) const
{
  std::vector<Entity *> res;
  for (auto y = yy - 20; y <= yy + 20; y += 10)
    for (auto x = xx - 20; x <= xx + 20; x += 10)
    {
      auto it = grid.find(getGridIdx(x, y));
      if (it == std::end(grid))
        continue;
      for (auto &&ent : it->second)
        if (std::hypot(xx - ent->getX(), yy - ent->getY()) < LookupRadios)
          res.push_back(ent);
    }
  return res;
}

void World::remove(Entity &ent)
{
  if (auto &&aEnt = dynamic_cast<ActiveEntity *>(&ent))
    active.erase(aEnt);
  auto it = grid.find(getGridIdx(ent.getX(), ent.getY()));
  if (it != std::end(grid))
    it->second.erase(&ent);
  entities.erase(&ent);
}

void World::kill(Entity &ent)
{
  killArray.push_back(&ent);

  auto a = 0.0f;
  for (auto mat = ent.getMatter(); mat > 0;)
  {
    auto st = add(std::make_unique<Stone>(*this,
                                          2.0f * a * sin(a * 2 * 3.1415926f / 12.0f) + ent.getX(),
                                          2.0f * a * cos(a * 2 * 3.1415926f / 12.0f) + ent.getY()));
    mat -= st->getMatter();
    ++a;
  }
}
