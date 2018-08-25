#include "bot.hpp"
#include "bot_class.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <SDL.h>
#include <shade/obj.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

Bot::Bot(World &world, float x, float y) : Entity(world, x, y) {}

void Bot::draw(Var<glm::mat4> &mvp)
{
  mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y))) *
        glm::rotate(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  mvp.update();
  auto &&anim = world->botClass->busyAnim;
  anim[SDL_GetTicks() * 30 / 1000 % anim.size()]->draw();
}

void Bot::tick() {}
