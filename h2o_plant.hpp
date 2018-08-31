#pragma once

#include "entity.hpp"

class H2OPlant: public Entity
{
public:
  H2OPlant(World &, int ttl, float x, float y);
  void draw() override;
  static const int Matter = 100;
  int getMatter() const override;
};
