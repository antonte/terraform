#include "o2_plant.hpp"

#include "world.hpp"
#include "terrain.hpp"
#include "rend.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

O2Plant::O2Plant(World &world, int ttl, float x, float y) : Entity(world, x, y)
{
  world.sched([this]() { this->world->kill(*this); }, ttl);
  ++world.o2Rate;
}

O2Plant::~O2Plant()
{
  --world->o2Rate;
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
