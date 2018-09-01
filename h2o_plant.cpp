#include "h2o_plant.hpp"

#include "terrain.hpp"
#include "world.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

H2OPlant::H2OPlant(World &world, int ttl, float x, float y) : Entity(world, x, y)
{
  world.sched([this]() { this->world->kill(*this); }, ttl);
  ++world.h2OPlantsNum;
}

H2OPlant::~H2OPlant()
{
  --world->h2OPlantsNum;
}

void H2OPlant::draw()
{
  world->shad->use();
  world->mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y)));
  world->mvp.update();
  world->h2OPlantObj->draw();
}

int H2OPlant::getMatter() const
{
  return Matter;
}
