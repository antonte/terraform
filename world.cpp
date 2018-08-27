#include "world.hpp"

#include "bot_class.hpp"
#include "entity.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include <coeff/coefficient_registry.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

World::World(Library &lib)
  : botClass(std::make_unique<BotClass>(lib)),
    stoneClass(std::make_unique<StoneClass>(lib)),
    terrain(std::make_unique<Terrain>(lib)),
    mvp("mvp"),
    proj("proj", glm::perspective(glm::radians(45.0f), 1.0f * Width / Height, 0.1f, 1000.0f)),
    view("view"),
    shad(std::make_unique<ShaderProgram>("shad", "shad", mvp, proj, view)),
    botShad(std::make_unique<ShaderProgram>("bot",
                                            "bot",
                                            mvp,
                                            proj,
                                            view,
                                            botClass->energy,
                                            botClass->matter))
{
}

World::~World() {}

static int getGridIdx(int  x, int y)
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
}

void World::add(std::unique_ptr<Entity> &&ent)
{
  auto entPtr = ent.get();
  grid[getGridIdx(entPtr->getX(), entPtr->getY())].insert(entPtr);
  if (entPtr->isActive())
    active.insert(entPtr);
  entities.emplace(entPtr, std::move(ent));
}

float World::getO2Level() const
{
  // TODO
  return 0.0f;
}

float World::getWaterLevel() const
{
  // TODO
  return 0.0f;
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
  auto it = grid.find(getGridIdx(ent.getX(), ent.getY()));
  if (it != std::end(grid))
    it->second.erase(&ent);
  entities.erase(&ent);
}
