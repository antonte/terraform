#pragma once
#include "entity.hpp"

class Bot : public Entity
{
public:
  Bot(World&, float x, float y);
  void draw(Var<glm::mat4> &mvp) override;
  void tick() override;
private:
};
