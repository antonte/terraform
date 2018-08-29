#pragma once

#include "entity.hpp"

class H2OPlant: public Entity
{
public:
  H2OPlant(World&, float x, float y);
  void draw() override;
};
