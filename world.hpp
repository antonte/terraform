#pragma once
#include <memory>

class BotClass;
class Library;
class StoneClass;

class World
{
public:
  World(Library &);
  ~World();
  std::unique_ptr<BotClass> botClass;
  std::unique_ptr<StoneClass> stoneClass;

  // std::unique_ptr<Terrain> terrain;
};
