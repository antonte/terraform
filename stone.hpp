#pragma once
#include "entity.hpp"

class World;

class Stone : public Entity
{
public:
  Stone(World&, float x, float y);
  void draw() override;
  void tick() override;
private:
  // level
};
