#pragma once
#include <memory>

class Library;
class Rend;
class World;

class Inst
{
public:
  Inst(Library&);
  ~Inst();
  std::unique_ptr<World> world;
  std::unique_ptr<Rend> rend;
};
