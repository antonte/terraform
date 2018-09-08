#pragma once
#include <vector>

class Library;
class Obj;

class StoneClass
{
public:
  StoneClass(Library&);
  ~StoneClass();
  std::vector<Obj *> level;
};
