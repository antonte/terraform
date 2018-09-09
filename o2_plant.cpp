#include "o2_plant.hpp"

#include "rend.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

O2Plant::O2Plant(World &world, int lifeSpan, int aProdRate, float x, float y)
  : Entity(world, x, y), prodRate(aProdRate)
{
  world.sched([this]() { this->world->kill(*this); }, lifeSpan);
  // TODO fix rounding issue
  world.o2Rate += prodRate / 100;
}

O2Plant::~O2Plant()
{
  // TODO fix rounding issue
  world->o2Rate -= prodRate / 100;
}

void O2Plant::draw(Rend &rend)
{
  rend.shad->use();
  rend.mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y)));
  rend.mvp.update();
  rend.o2PlantObj->draw();
}

int O2Plant::getMatter() const
{
  return Matter;
}
