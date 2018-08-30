#include "stone.hpp"
#include "stone_class.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

Stone::Stone(World &world, float x, float y) : Entity(world, x, y)
{
  direction = (rand() % 360) * 2.0f * 3.1415f / 360.0f;
}

void Stone::draw()
{
  world->shad->use();
  world->mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y))) *
        glm::rotate(direction, glm::vec3(0.0f, 0.0f, 1.0f));
  world->mvp.update();
  world->stoneClass->level[0]->draw();
}

int Stone::getMatter() const
{
  return Matter;
}
