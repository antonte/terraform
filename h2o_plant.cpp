#include "h2o_plant.hpp"

#include "world.hpp"
#include "terrain.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

H2OPlant::H2OPlant(World &world, float x, float y) : Entity(world, x, y) {}

void H2OPlant::draw()
{
  world->shad->use();
  world->mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y)));
  world->mvp.update();
  world->h2OPlantObj->draw();
}
