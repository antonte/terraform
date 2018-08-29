#pragma once

class ActiveEntity
{
public:
  virtual ~ActiveEntity() = default;
  virtual void tick() = 0;
};
