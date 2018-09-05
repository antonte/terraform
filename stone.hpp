#pragma once
#include "entity.hpp"

class World;
class Rend;

class Stone : public Entity
{
public:
  Stone(World&, float x, float y);
  void draw(Rend&) override final;
  int getMatter() const override;
  static const int Matter = 10;

private:
  // level
};
