#pragma once
#include "entity.hpp"

class World;

class Stone : public Entity
{
public:
  Stone(World&, float x, float y);
  void draw() override;
  int getMatter() const override;
  static const int Matter = 10;

private:
  // level
};
