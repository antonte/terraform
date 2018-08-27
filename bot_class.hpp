#pragma once
#include <vector>
#include <shade/var.hpp>

class Library;
class Obj;

class BotClass
{
public:
  BotClass(Library &);
  std::vector<Obj *> idleAnim;
  std::vector<Obj *> walkAnim;
  std::vector<Obj *> busyAnim;
  Var<float> energy;
  Var<float> matter;
};
