#include "world.hpp"

#include "bot_class.hpp"
#include "entity.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include <coeff/coefficient_registry.hpp>

World::World(Library &lib)
  : botClass(std::make_unique<BotClass>(lib)),
    stoneClass(std::make_unique<StoneClass>(lib)),
    terrain(std::make_unique<Terrain>(lib))
{
}

World::~World() {}

static int getGridIdx(int  x, int y)
{
  return (x + Terrain::Width / 2) / 10 + ((y + Terrain::Height / 2) / 10) * (Terrain::Width / 10);
}

void World::draw(Var<glm::mat4> &mvp, int minX, int maxX, int minY, int maxY)
{
  terrain->draw(mvp, minX, maxX, minY, maxY);
  for (auto y = minY - 10; y < maxY + 10; y += 10)
    for (auto x = minX - 10; x < maxX + 10; x += 10)
    {
      auto it = grid.find(getGridIdx(x, y));
      if (it == std::end(grid))
        continue;
      for (auto &&ent : it->second)
        ent->draw(mvp);
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
