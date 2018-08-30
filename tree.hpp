#pragma once

#include "entity.hpp"


class Tree: public Entity
{
public:
  Tree(World&, float x, float y);
  void draw() override;
  static const int Matter = 100;
  int getMatter() const override;
};
