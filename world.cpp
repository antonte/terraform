#include "world.hpp"

#include "bot_class.hpp"
#include "entity.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"

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
  // for (auto&& ent: entities)
  //   ent->draw(mvp);
}

void World::add(std::unique_ptr<Entity> &&ent)
{
  grid[getGridIdx(ent->getX(), ent->getY())].insert(ent.get());
  entities.push_back(std::move(ent));
}

