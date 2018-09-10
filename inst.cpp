#include "inst.hpp"

#include "rend.hpp"
#include "world.hpp"

Inst::Inst(sdl::Renderer &sdlRend)
  : rend(std::make_unique<Rend>(sdlRend)), world(std::make_unique<World>(*rend->lib))
{
}

Inst::~Inst() {}
