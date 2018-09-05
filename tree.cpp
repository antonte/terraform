#include "tree.hpp"

#include "pi.hpp"
#include "rend.hpp"
#include "terrain.hpp"
#include "world.hpp"
#include <shade/obj.hpp>
#include <shade/shader_program.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

Tree::Tree(World &world, float x, float y)
  : Entity(world, x, y),
    dob(world.getNow()),
    ttl(100 + 360000 * world.getO2Level() * world.getH2OLevel())
{
  world.sched([this]() { this->world->kill(*this); }, ttl);
  unSched = world.sched.schedWithUnSched([this]() { tryToReproduce(); }, rand() % 900 + 100);
  ++world.treesNum;
}

Tree::~Tree()
{
  --world->treesNum;
}

void Tree::draw(Rend &rend)
{
  auto age = 0.3f + 3.0f * (world->getNow() - dob) / 360000;
  rend.shad->use();
  rend.mvp = glm::translate(glm::vec3(x, y, world->terrain->getZ(x, y))) *
               glm::scale(glm::vec3(age, age, age));
  rend.mvp.update();
  rend.treeObj->draw();
}

int Tree::getMatter() const
{
  return Matter;
}

void Tree::tryToReproduce()
{
  for (auto &&ent : world->getAround(x, y, 19.0f))
  {
    if (ent == this)
      continue;
    if (dynamic_cast<Tree *>(ent))
      return;
  }
  if (rand() % 4500 <
      (100 * world->getO2Level() * world->getH2OLevel()))
  {
    auto dir = rand() % 360;
    world->add<Tree>(x + 20 * cos(dir * 2.0f * PI / 360.0f),
                     y + 20 * sin(dir * 2.0f * PI / 360.0f));
  }
  unSched = world->sched.schedWithUnSched([this]() { tryToReproduce(); }, rand() % 900 + 100);
}
