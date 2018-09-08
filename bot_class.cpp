#include "bot_class.hpp"

#include <shade/library.hpp>

BotClass::BotClass(Library &lib) : energy("energy"), matter("matter")
{
  for (int f = 21; f <= 30; ++f)
    busyAnim.push_back(lib.getObj("bot_busy_0000" + std::to_string(f), "bot"));
  for (int f = 31; f <= 50; ++f)
    idleAnim.push_back(lib.getObj("bot_idle_0000" + std::to_string(f), "bot"));
  for (int f = 1; f <= 9; ++f)
    walkAnim.push_back(lib.getObj("bot_walk_00000" + std::to_string(f), "bot"));
  walkAnim.push_back(lib.getObj("bot_walk_000010", "bot"));
}
