#pragma once

#include "entity.hpp"


class O2Plant: public Entity
{
public:
  O2Plant(World&, float x, float y);
  void draw() override;
};
