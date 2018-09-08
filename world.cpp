#include "world.hpp"

#include "active_entity.hpp"
#include "button.hpp"
#include "entity.hpp"
#include "pi.hpp"
#include "rend.hpp"
#include "stone.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include "ui.hpp"
#include <coeff/coefficient_registry.hpp>
#include <log/log.hpp>
#include <shade/array_buffer.hpp>
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>
#include <shade/text.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

COEFF(MapXK, 0.1f);
COEFF(MapYK, 0.1f);
COEFF(MapYBalance, 50);

World::World(Library &lib) : terrain(std::make_unique<Terrain>(lib)) {}

World::~World() {}

static int getGridIdx(int x, int y)
{
  return (x + Terrain::Width / 2) / 10 + ((y + Terrain::Height / 2) / 10) * (Terrain::Width / 10);
}

void World::draw(Rend &rend, float camX, float camY, float camZ)
{
  rend.o2Level = getO2Level();
  rend.h2OLevel = getH2OLevel();
  int minX = camX - camZ * MapXK;
  int maxX = camX + camZ * MapXK;
  int minY = camY - camZ * MapYK * MapYBalance / 100;
  int maxY = camY + camZ * MapYK;

  rend.view = glm::lookAt(glm::vec3(camX, camY - camZ, camZ),
                     glm::vec3(camX, camY, 0.0f), // and looks at the origin
                     glm::vec3(0, 0, 1));
  terrain->draw(rend, minX, maxX, minY, maxY);
  rend.stoneMvps.clear();
  for (auto y = minY - 10; y < maxY + 10; y += 10)
    for (auto x = minX - 10; x < maxX + 10; x += 10)
    {
      auto it = grid.find(getGridIdx(x, y));
      if (it == std::end(grid))
        continue;
      for (auto &&ent : it->second)
        ent->draw(rend);
    }

  {
    rend.stoneShad->use();
    ArrayBuffer mvpsAb(rend.stoneMvps, 3);
    mvpsAb.activate();
    glVertexAttribDivisor(3, 1);
    rend.stoneClass->level[0]->drawInstanced(rend.stoneMvps.size());
  }

  // draw water
  rend.mvp = glm::translate(glm::vec3(0.0f, 0.0f, (getH2OLevel() * 10.0f - 15.0f)));
  rend.time = SDL_GetTicks();
  rend.waterShad->use();
  rend.waterMesh->activate();
  glDrawArrays(GL_TRIANGLES, 0, 6);
}

Entity *World::internalAdd(std::unique_ptr<Entity> &&ent)
{
  auto entPtr = ent.get();
  grid[getGridIdx(entPtr->getX(), entPtr->getY())].insert(entPtr);
  if (auto e = dynamic_cast<ActiveEntity *>(entPtr))
    active.insert(e);
  return entities.emplace(entPtr, std::move(ent)).first->second.get();
}

float World::getO2Level() const
{
  return realO2Level / 100000000.0f;
}

float World::getH2OLevel() const
{
  return realH2OLevel / 100000000.0f;
}

float World::getTreeLevel() const
{
  return treesNum / 1000.0f;
}

void World::tick()
{
  money += getIncome() / 100;
  realO2Level += o2Rate;
  realH2OLevel += h2ORate;
  for (auto &&ent : active)
    ent->tick();

  sched.tick();
  ++now;
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

std::vector<Entity *> World::getAround(float xx, float yy, float rad) const
{
  std::vector<Entity *> res;
  for (auto y = yy - 20; y <= yy + 20; y += 10)
    for (auto x = xx - 20; x <= xx + 20; x += 10)
    {
      auto it = grid.find(getGridIdx(x, y));
      if (it == std::end(grid))
        continue;
      for (auto &&ent : it->second)
        if (std::hypot(xx - ent->getX(), yy - ent->getY()) < ((rad < 0) ? LookupRadios : rad))
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
  sched([this, &ent]() { remove(ent); });
  auto a = 0.0f;
  for (auto mat = ent.getMatter(); mat > 0;)
  {
    auto st = add<Stone>(2.0f * a * sin(a * 2 * PI / 12.0f) + ent.getX(),
                         2.0f * a * cos(a * 2 * PI / 12.0f) + ent.getY());
    mat -= st->getMatter();
    ++a;
  }
}


int World::getNow() const
{
  return now;
}

bool World::isAlive(const Entity &ent) const
{
  return entities.find(&ent) != std::end(entities);
}

int64_t World::getIncome() const
{
  return (realO2Level + realH2OLevel + treesNum) / 3ll;
}
