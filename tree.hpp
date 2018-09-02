#pragma once
#include "entity.hpp"
#include "sched.hpp"

class Tree : public Entity
{
public:
  Tree(World &, float x, float y);
  ~Tree();
  void draw() override;
  static const int Matter = 100;
  int getMatter() const override;

private:
  void tryToReproduce();
  UnSched unSched;
  int dob;
  int ttl;
};
