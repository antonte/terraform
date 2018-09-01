#pragma once
#include "entity.hpp"

class Tree : public Entity
{
public:
  Tree(World &, int ttl, float x, float y);
  ~Tree();
  void draw() override;
  static const int Matter = 100;
  int getMatter() const override;
};
