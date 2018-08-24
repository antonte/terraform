#include "entity.hpp"

Entity::Entity(World &aWorld, float aX, float aY) : world(&aWorld), x(aX), y(aY) {}

float Entity::getX() const
{
  return x;
}

float Entity::getY() const
{
  return y;
}
