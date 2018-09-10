#pragma once
#include <memory>

class Library;
class Rend;
class World;

namespace sdl
{
  class Renderer;
}

class Inst
{
public:
  Inst(sdl::Renderer &);
  ~Inst();
  std::unique_ptr<Rend> rend;
  std::unique_ptr<World> world;
};
