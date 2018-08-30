#include "o2_plant.hpp"

#include "world.hpp"
#include "terrain.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

O2Plant::O2Plant(World &world, float x, float y) : Entity(world, x, y) {}

void O2Plant::draw()
{
  world->shad->use();
  world->mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y)));
  world->mvp.update();
  world->o2PlantObj->draw();
}

int O2Plant::getMatter() const
{
  return Matter;
}
