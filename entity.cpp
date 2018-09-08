#include "entity.hpp"

#include "terrain.hpp"
#include <algorithm>

Entity::Entity(World &aWorld, float aX, float aY)
  : world(&aWorld),
    x(std::max(std::min(aX, Terrain::Width / 2.0f), -Terrain::Width / 2.0f)),
    y(std::max(std::min(aY, Terrain::Height / 2.0f), -Terrain::Height / 2.0f))
{
}

float Entity::getX() const
{
  return x;
}

float Entity::getY() const
{
  return y;
}

void Entity::setXY(float aX, float aY)
{
  x = aX;
  y = aY;
}
