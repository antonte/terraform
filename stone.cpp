#include "stone.hpp"
#include "stone_class.hpp"
#include "world.hpp"
#include <shade/obj.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

Stone::Stone(World &world, float x, float y) : Entity(world, x, y)
{
  direction = (rand() % 360) * 2.0f * 3.1415f / 360.0f;
}

void Stone::draw(Var<glm::mat4> &mvp)
{
  mvp = glm::translate(glm::vec3(x, y, 0.0f)) * glm::rotate(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  mvp.update();
  world->stoneClass->level[0]->draw();
}

void Stone::tick() {}
