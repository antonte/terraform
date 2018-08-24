#pragma once
#include <glm/mat4x4.hpp>
#include <shade/var.hpp>

class World;

class Entity
{
public:
  Entity(World &, float x, float y);
  virtual ~Entity() = default;
  float getX() const;
  float getY() const;
  virtual void draw(Var<glm::mat4> &mvp) = 0;
  virtual void tick() = 0;

protected:
  World *world;
  float x;
  float y;
  float direction = 0.0f;
};
