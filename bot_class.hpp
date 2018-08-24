#pragma once
#include <vector>

class Library;
class Obj;

class BotClass
{
public:
  BotClass(Library &);
  std::vector<Obj *> idleAnim;
  std::vector<Obj *> walkAnim;
  std::vector<Obj *> busyAnim;
};
