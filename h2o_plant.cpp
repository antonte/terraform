#include "h2o_plant.hpp"

#include "rend.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

H2OPlant::H2OPlant(World &world, int ttl, float x, float y) : Entity(world, x, y)
{
  world.sched([this]() { this->world->kill(*this); }, ttl);
  ++world.h2ORate;
}

H2OPlant::~H2OPlant()
{
  --world->h2ORate;
}

void H2OPlant::draw(Rend &rend)
{
  rend.shad->use();
  rend.mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y)));
  rend.mvp.update();
  rend.h2OPlantObj->draw();
}

int H2OPlant::getMatter() const
{
  return Matter;
}
