#pragma once
#include "entity.hpp"

class O2Plant: public Entity
{
public:
  O2Plant(World&, int ttl, float x, float y);
  ~O2Plant();
  void draw() override;
  static const int Matter = 100;
  int getMatter() const override;
};
