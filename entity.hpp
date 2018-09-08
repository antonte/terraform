#pragma once

class Rend;
class World;

class Entity
{
public:
  Entity(World &, float x, float y);
  virtual ~Entity() = default;
  float getX() const;
  float getY() const;
  void setXY(float x, float y);
  virtual void draw(Rend &) = 0;
  virtual int getMatter() const = 0;

protected:
  World *world;
  float x;
  float y;
  float direction = 0.0f;
};
