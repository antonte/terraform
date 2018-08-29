#pragma once
#include "entity.hpp"

class World;

class Stone : public Entity
{
public:
  Stone(World&, float x, float y);
  void draw() override;
private:
  // level
};
