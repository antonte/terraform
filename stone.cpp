#include "stone.hpp"

#include "pi.hpp"
#include "rend.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <cstdlib>

Stone::Stone(World &world, float x, float y) : Entity(world, x, y)
{
  direction = (rand() % 360) * 2.0f * PI / 360.0f;
}

void Stone::draw(Rend& rend)
{
  rend.stoneMvps.push_back(glm::vec3(x, y, world->terrain->getZ(x, y)));
}

int Stone::getMatter() const
{
  return Matter;
}
