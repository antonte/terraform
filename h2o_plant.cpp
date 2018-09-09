#include "h2o_plant.hpp"

#include "rend.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <shade/shader_program.hpp>
#include <shade/obj.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

H2OPlant::H2OPlant(World &world, int lifeSpan, int aProdRate, float x, float y)
  : Entity(world, x, y), prodRate(aProdRate)
{
  world.sched([this]() { this->world->kill(*this); }, lifeSpan);
  // TODO fix rounding issue
  world.h2ORate += prodRate / 100;
}

H2OPlant::~H2OPlant()
{
  // TODO fix rounding issue
  world->h2ORate -= prodRate / 100;
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
