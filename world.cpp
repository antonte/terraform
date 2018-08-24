#include "world.hpp"

#include "bot_class.hpp"
#include "stone_class.hpp"

World::World(Library &lib)
  : botClass(std::make_unique<BotClass>(lib)), stoneClass(std::make_unique<StoneClass>(lib))
{
}

World::~World() {}
